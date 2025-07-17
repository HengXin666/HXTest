#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

template <typename T>
auto constexpr _log(T v) {
    std::size_t n = 0;
    while (v) {
        ++n;
        v >>= 1;
    }
    return n;
}

// FNV1a哈希：简单、constexpr兼容、适合小型哈希
constexpr uint64_t fnv1aHash(std::string_view s,
                             uint64_t seed = 0xcbf29ce484222325) {
    uint64_t hash = seed;
    for (char c : s) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 0x100000001b3ull;
    }
    return hash;
}

struct Entry {
    std::string_view key;
    std::size_t value;
};

template <size_t N, size_t BucketCount = 2 * N>
struct FMHMap {
    // 存放值：value数组
    std::array<std::size_t, N> slots{};
    // 存放键：key数组（用于查重）
    std::array<std::string_view, N> keys{};
    // 每个桶的偏移量（用于冲突消解）
    std::array<std::size_t, BucketCount> offsets{};

    // 构造时编译期生成FMH表
    constexpr FMHMap(std::array<Entry, N> init) {
        build(init);
    }

    // 查找操作：运行期 O(1)
    constexpr std::size_t lookup(std::string_view key) const {
        uint64_t h1 = fnv1aHash(key) % BucketCount; // 一级hash：决定是哪个桶
        uint64_t h2 =
            fnv1aHash(key, 0xdeadbeefcafebabe); // 二级hash：用于桶内偏移
        size_t idx = (h2 + offsets[h1]) % N;    // 实际查表位置

        // idx 需要保证是 所有的 key 除了自己, 其他的 key 都是不同的
        // 即记 getIdx = (h2 + offset[h1() % BN]) % N
        // 有 arr 内的 key, getIdx 互不相等
        /*
            其中, h1() % BN, h2 都是已知数, 计为 a, b; getIdx 记 f
            有 f = (b + offset[a]) % N
            因此能改变的 只有 offset[a], 故可以枚举 offset[a] in [0, n - 1]
            可是他的变化, 也会导致同一个桶的其他数也变化, 不会引起新的冲突吗
        */ 

        if (keys[idx] == key) {
            return slots[idx];
        }

        return static_cast<std::size_t>(-1); // 未找到
    }

private:
    // 编译期构建hash表
    constexpr void build(std::array<Entry, N> init) {
        std::array<size_t, N> used{};
        std::array<size_t, BucketCount> bucketSize{};

        // 统计桶大小
        for (auto&& e : init) {
            size_t b = fnv1aHash(e.key) % BucketCount;
            bucketSize[b]++;
        }

        // 创建桶信息和排序
        struct BucketInfo {
            size_t index;
            size_t size;
        };
        std::array<BucketInfo, BucketCount> buckets{};
        for (size_t i = 0; i < BucketCount; i++) {
            buckets[i] = {i, bucketSize[i]};
        }
        std::sort(buckets.begin(), buckets.end(), [](const BucketInfo& a, const BucketInfo& b) {
            return a.size > b.size;
        });

        // 大桶先处理
        for (auto& bucket : buckets) {
            if (bucket.size == 0)
                continue;

            size_t b = bucket.index;
            size_t attempt = 0;
            bool success = false;

            while (!success) {
                success = true;

                for (auto&& e : init) {
                    if (fnv1aHash(e.key) % BucketCount != b)
                        continue;

                    size_t h2 = fnv1aHash(e.key, 0xdeadbeefcafebabe);
                    size_t idx = (h2 + attempt) % N;

                    if (used[idx]) {
                        success = false;
                        break;
                    }
                }

                if (success) {
                    offsets[b] = attempt;

                    for (auto&& e : init) {
                        if (fnv1aHash(e.key) % BucketCount != b)
                            continue;

                        size_t h2 = fnv1aHash(e.key, 0xdeadbeefcafebabe);
                        size_t idx = (h2 + attempt) % N;

                        used[idx] = 1;
                        keys[idx] = e.key;
                        slots[idx] = e.value;
                    }
                } else {
                    ++attempt;
                }
            }
        }
    }
};

int main() {
    constexpr auto map = FMHMap{std::array{
        Entry{"id", 0},
        Entry{"i11d", 0},
        Entry{"name", 1},
        Entry{"value", 2},
        Entry{"time", 3},
        Entry{"timesdfa2", 3},
        Entry{"tiasdfsdfme3", 3},
        Entry{"time4", 3},
        Entry{"time5", 3},
        Entry{"tim6adsfae", 3},
        Entry{"timeasdf8", 3},
        Entry{"t7iasadsfdfme", 3},
        Entry{"tiasdfasdf7m1e", 3},
        Entry{"tiasdfasd7m1e", 3},
        Entry{"sdfasdfti7m1e", 3},
        Entry{"tfasdfi7asdfasdfm1e", 3},
        Entry{"ti7m1asdfasdfe", 3},
        Entry{"ti7m1dsafasfde", 3},
        Entry{"ti7masdf1e", 3},
        Entry{"tiasdfsdaf7m1e", 3},
        Entry{"ti7am1e", 3},
        Entry{"ti7sadfasdfm1e", 3},
        Entry{"ti7dfasdfm1e", 3},
        Entry{"ti7m1asdfasdfe", 3},
        Entry{"ti7asdfasdfm1e", 3},
        Entry{"tiaadfadsf7m1e", 3},
        Entry{"tisd7m1e", 3},
        Entry{"ti7adfadsfmsafasdf1e", 3},
        Entry{"tiadfadf7m21e", 3},
        Entry{"ti37m1e", 3},
        Entry{"ti47m1e", 3},
        Entry{"ti71m1e", 3},
        Entry{"ti71m1aasdasdsdasde", 3},
        Entry{"ti71m1asda1sde", 3},

        Entry{"aaasdaid", 0},
        Entry{"aaasdaname", 1},
        Entry{"aaasdavalue", 2},
        Entry{"aaasdatime", 3},
        Entry{"aaasdatimesdfa2", 3},
        Entry{"aaasdatiasdfsdfme3", 3},
        Entry{"aaasdatime4", 3},
        Entry{"aaasdatime5", 3},
        Entry{"aaasdatim6adsfae", 3},
        Entry{"aaasdatimeasdf8", 3},
        Entry{"aaasdat7iasadsfdfme", 3},
        Entry{"aaasdatiasdfasdf7m1e", 3},
        Entry{"aaasdatiasdfasd7m1e", 3},
        Entry{"aaasdasdfasdfti7m1e", 3},
        Entry{"aaasdatfasdfi7asdfasdfm1e", 3},
        Entry{"aaasdati7m1asdfasdfe", 3},
        Entry{"aaasdati7m1dsafasfde", 3},
        Entry{"aaasdati7masdf1e", 3},
        Entry{"aaasdatiasdfsdaf7m1e", 3},
        Entry{"aaasdati7am1e", 3},
        Entry{"aaasdati7sadfasdfm1e", 3},
        Entry{"aaasdati7dfasdfm1e", 3},
        Entry{"11", 3},
        Entry{"111", 3},
        Entry{"aaasdati7asdfasdfm1e", 3},
        Entry{"aaasdatiaadfadsf7m1e", 3},
        Entry{"aaasdatisd7m1e", 3},
        Entry{"aaasdati7adfadsfmsafasdf1e", 3},
        Entry{"aaasdatiadfadf7m21e", 3},
        Entry{"aaasdati71m1aasdasdsdasde", 3},
        Entry{"aaasdati71m1asda1sde", 3},
        Entry{"aaas1dati7111m1asda221sde", 3},
        Entry{"aaas1dati7111m1asd2a1sde", 3},
        Entry{"aaas1dati71111m1a222sda1sde", 3},
    }};

    static_assert(map.lookup("id") == 0);
    static_assert(map.lookup("name") == 1);
    static_assert(map.lookup("value") == 2);
    static_assert(map.lookup("time") == 3);
    static_assert(map.lookup("unknown") == static_cast<std::size_t>(-1));
}