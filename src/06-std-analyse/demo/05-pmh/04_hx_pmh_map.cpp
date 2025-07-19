#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>

auto constexpr nextHighestPowerOfTwo(std::size_t v) {
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    v--;
    for (std::size_t i = 1; i < sizeof(std::size_t) * 8; i <<= 1)
        v |= v >> i;
    v++;
    return v;
}

template <class T>
auto constexpr _log(T v) {
    std::size_t n = 0;
    while (v > 1) {
        n += 1;
        v >>= 1;
    }
    return n;
}

struct XorShift32 {
    uint32_t a;

    constexpr XorShift32(size_t seed = 0) 
        : a(static_cast<uint32_t>(seed + 1)) 
    {}

    using result_type = uint32_t;

    constexpr uint32_t operator()() noexcept {
        uint32_t x = a;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return a = x;
    }
};

using default_prg_t = XorShift32;

template <class T, std::size_t N>
class cvector {
    T _data[N] = {}; // 标量类型 T 的零初始化，
                    // 否则为 default-initialized
    std::size_t _dsize = 0;

public:
    // Container typdefs
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Constructors
    constexpr cvector(void) = default;
    constexpr cvector(size_type count, const T& value) : _dsize(count) {
        for (std::size_t i = 0; i < N; ++i)
            _data[i] = value;
    }

    // Iterators
    constexpr iterator begin() noexcept {
        return _data;
    }
    constexpr iterator end() noexcept {
        return _data + _dsize;
    }

    // Capacity
    constexpr size_type size() const {
        return _dsize;
    }

    // Element access
    constexpr reference operator[](std::size_t index) {
        return _data[index];
    }
    constexpr const_reference operator[](std::size_t index) const {
        return _data[index];
    }

    constexpr reference back() {
        return _data[_dsize - 1];
    }
    constexpr const_reference back() const {
        return _data[_dsize - 1];
    }

    // Modifiers
    constexpr void push_back(const T& a) {
        _data[_dsize++] = a;
    }
    constexpr void push_back(T&& a) {
        _data[_dsize++] = std::move(a);
    }
    constexpr void pop_back() {
        --_dsize;
    }

    constexpr void clear() {
        _dsize = 0;
    }
};

// 检查项目是否出现在 cvector 中
template <class T, size_t N>
constexpr bool allDifferentFrom(cvector<T, N>& data, T& a) {
    for (std::size_t i = 0; i < data.size(); ++i)
        if (data[i] == a)
            return false;
    return true;
}

// 表示数据项数组的索引或要使用的种子一个哈希器。
// 种子必须具有 1 的高位, 值必须具有 0 的高位。
struct SeedOrIndex {
    using value_type = uint64_t;

private:
    inline static constexpr value_type MINUS_ONE 
        = (std::numeric_limits<value_type>::max)();

    inline static constexpr value_type HIGH_BIT 
        = ~(MINUS_ONE >> 1);

    value_type _value = 0;

public:
    constexpr value_type value() const {
        return _value;
    }
    constexpr bool isSeed() const {
        return _value & HIGH_BIT;
    }

    constexpr SeedOrIndex(bool is_seed, value_type value) 
        : _value{is_seed ? (value | HIGH_BIT) 
                         : (value & ~HIGH_BIT)} 
    {}

    constexpr SeedOrIndex() = default;
};

template <size_t M>
struct PmhBuckets {
    // 第 0 步：存储桶最大值为 2 * sqrt M
    // TODO： 想出这个理由，不应该是 O（log M） 吗？
    static constexpr auto bucket_max = 2 * (1u << (_log(M) / 2));

    using bucket_t = cvector<std::size_t, bucket_max>;
    std::array<bucket_t, M> buckets; // 桶 [i][j]
                                     // buckets[i] 表示第 i + 1 个桶
                                     // buckets[i][j] 表示第 i + 1 个桶的第 j + 1 个元素,
                                     // 其映射到 items 索引为 buckets[i][j] 的元素
    uint64_t seed; // 计算桶索引使用的哈希种子
public:
    // 表示对存储桶的引用。之所以使用此方法，是因为存储桶
    // 必须进行排序，但存储桶很大(元素是cvector, 显然不能这样(编译期一般是拷贝或者交换))，因此比排序引用慢
    struct BucketRef {
        unsigned index;             // 原本元素的索引
        const bucket_t* ptr;

        // 转发 bucket 的一些接口
        using value_type = typename bucket_t::value_type;
        using const_iterator = typename bucket_t::const_iterator;

        constexpr auto size() const {
            return ptr->size();
        }
        constexpr const auto& operator[](std::size_t idx) const {
            return (*ptr)[idx];
        }
    };

    // 为每个存储桶创建一个 bucket_ref
    template <std::size_t... Is>
    std::array<BucketRef, M> constexpr makeBucketRefs(
        std::index_sequence<Is...>
    ) const {
        return {{BucketRef{Is, &buckets[Is]}...}};
    }

    // 为每个存储桶创建一个bucket_ref并按大小对它们进行排序
    std::array<BucketRef, M> constexpr getSortedBuckets() const {
        std::array<BucketRef, M> result{
            makeBucketRefs(std::make_index_sequence<M>())
        };
        // 编译期排序, 按照 桶大小 从大到小 排序
        std::sort(result.begin(), result.end(), [](BucketRef const& b1, BucketRef const& b2) {
            return b1.size() > b2.size();
        });
        return result;
    }
};

template <size_t M, class Item, size_t N, class Hash, class Key, class PRG>
PmhBuckets<M> constexpr makePmhBuckets(const std::array<Item, N>& items,
                                       Hash const& hash, Key const& key,
                                       PRG& prg) {
    using result_t = PmhBuckets<M>;
    result_t result{};
    bool rejected = false;
    // 继续作，直到放置完所有项目，且不超过 bucket_max
    while (1) {
        for (auto& b : result.buckets) {
            b.clear();
        }
        result.seed = prg(); // 生成一个种子
        rejected = false;
        for (std::size_t i = 0; i < N; ++i) {
            // 元素映射到桶
            auto& bucket = result.buckets[hash(key(items[i]), static_cast<size_t>(result.seed)) % M];
            if (bucket.size() >= result_t::bucket_max) {
                rejected = true;
                break;
            }
            bucket.push_back(i); // bucket 记录桶中映射的元素索引
        }
        if (!rejected) {
            return result;
        }
    }
}

// 表示 pmh 算法创建的完美哈希函数
template <std::size_t M, class Hasher>
struct PmhTables {
    uint64_t _first_seed;
    std::array<SeedOrIndex, M> _first_table;  // 记录种子或者索引
    std::array<std::size_t, M> _second_table; // 第二层哈希表
    Hasher _hash;

    template <typename KeyType>
    constexpr std::size_t lookup(const KeyType& key) const {
        return lookup(key, _hash);
    }

    // 查找给定的键, 以在 carray<Item， N 中找到其预期索引>
    // 总是返回有效的索引, 之后必须使用 KeyEqual 测试来确认。
    template <typename KeyType, typename HasherType>
    constexpr std::size_t lookup(const KeyType& key,
                                 const HasherType& hasher) const {
        auto const d =
            _first_table[hasher(key, static_cast<size_t>(_first_seed)) % M];
        // 如果不是种子就直接返回索引
        if (!d.isSeed()) {
            return static_cast<std::size_t>(d.value());
        } // 这是缩小 uint64 -> size_t 但应该没问题
        else {
            // 如果是种子, 就作为种子查第二个哈希表, 得到索引
            return _second_table[
                hasher(key, 
                       static_cast<std::size_t>(d.value())
                ) % M];
        }
    }
};

// 为给定的项目、哈希函数、prg 等制作 pmh 表。
template <std::size_t M, class Item, std::size_t N, class Hash, class Key,
          class PRG>
PmhTables<M, Hash> constexpr makePmhTables(const std::array<Item, N>& items,
                                           Hash const& hash, Key const& key,
                                           PRG prg) {
    // 第 1 步：将所有密钥放入存储桶中
    auto stepOne = makePmhBuckets<M>(items, hash, key, prg);

    // 第 2 步：对存储桶进行排序，以首先处理项目最多的存储桶。
    auto buckets = stepOne.getSortedBuckets();

    // G 成为生成的 pmh 函数中的第一个哈希表
    std::array<SeedOrIndex, M> G; // 默认构造为 “index 0”
                                  // 索引 或者 哈希种子

    // H 成为生成的 pmh 函数中的第二个哈希表
    constexpr std::size_t UNUSED = (std::numeric_limits<std::size_t>::max)();
    std::array<std::size_t, M> H; // 纯索引
    H.fill(UNUSED);

    // 第 3 步：将存储桶中的项目映射到哈希表中。
    for (const auto& bucket : buckets) { // 获取每个桶
        auto const bsize = bucket.size();

        // 桶大小为 1
        if (bsize == 1) {
            // 在 G 中存储 （单个） 项的索引
            G[bucket.index] = {false, static_cast<uint64_t>(bucket[0])}; // 是索引
        } else if (bsize > 1) {
            // 桶大小不为 1, 那需要保证桶元素全部可以映射到 二级哈希表 H 中的 空的 独立的位置 (也就是不冲突)
            // 反复尝试不同的 H 或 d, 直到找到一个哈希函数将桶中的所有物品放入空闲槽中
            SeedOrIndex d{true, prg()}; // d 是种子

            // 记录桶元素 索引到的 二级哈希表 H 中的位置的索引
            // 因为会匹配失败, 所以需要反悔, 因此我们可以分开记录, 这样就只需要 bucketSlots.clear() 即可
            cvector<std::size_t, decltype(stepOne)::bucket_max> bucketSlots;

            while (bucketSlots.size() < bsize) {
                // 计算桶中, 第 idx = bucketSlots.size() 个元素索引到的 items 的 key 的哈希值
                auto slot = hash(
                    key(
                        items[bucket[bucketSlots.size()]]
                    ),
                        static_cast<size_t>(d.value())
                    ) % M;

                // 如果这个哈希值映射到 [0, M) 的 二级哈希表 H 中, 
                // 如果这个位置是空的 或者 没有被在本次记录中之前的桶元素索引到
                if (H[slot] != UNUSED || !allDifferentFrom(bucketSlots, slot)) {
                    // 如果不满足, 就清空记录, 然后换哈希函数 (也就是换哈希种子)
                    bucketSlots.clear();
                    d = {true, prg()};
                    continue;
                }
                bucketSlots.push_back(slot);
            }

            // 将成功的种子放入 G 中, 并将索引放入其槽中的项目
            G[bucket.index] = d; // 给一级哈希表记录 种子
            // 二级哈希表记录上本次记录
            for (std::size_t i = 0; i < bsize; ++i)
                H[bucketSlots[i]] = bucket[i];
        }
    }

    // H 表中任何未使用的条目都必须更改为零。
    // 这是因为哈希处理不应失败或返回越界条目。
    // 将用户提供的 KeyEqual 应用于查询并且
    // 通过哈希找到的 key。将此类查询发送到 0 不会有什么坏处。
    for (std::size_t i = 0; i < M; ++i)
        if (H[i] == UNUSED)
            H[i] = 0;

    return {stepOne.seed, G, H, hash};
}

struct GetKey {
    template <class KV>
    constexpr auto const& operator()(KV const& kv) const {
        return kv.first;
    }
};

template <class T = void>
struct elsa {
    static_assert(std::is_integral<T>::value || std::is_enum<T>::value,
                  "only supports integral types, specialize for other types");

    constexpr std::size_t operator()(T const& value, std::size_t seed) const {
        std::size_t key = seed ^ static_cast<std::size_t>(value);
        key = (~key) + (key << 21); // key = (key << 21) - key - 1;
        key = key ^ (key >> 24);
        key = (key + (key << 3)) + (key << 8); // key * 265
        key = key ^ (key >> 14);
        key = (key + (key << 2)) + (key << 4); // key * 21
        key = key ^ (key >> 28);
        key = key + (key << 31);
        return key;
    }
};

template <>
struct elsa<void> {
    template <class T>
    constexpr std::size_t operator()(T const& value, std::size_t seed) const {
        return elsa<T>{}(value, seed);
    }
};

template <typename String>
constexpr std::size_t hashString(const String& value) {
    std::size_t d = 5381;
    for (const auto& c : value) 
        d = d * 33 + static_cast<size_t>(c);
    return d;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// 根据实验设置删除最低位。
template <typename String>
constexpr std::size_t hashString(const String& value, std::size_t seed) {
    std::size_t d = (0x811c9dc5 ^ seed) * static_cast<size_t>(0x01000193);
    for (const auto& c : value)
        d = (d ^ static_cast<size_t>(c)) * static_cast<size_t>(0x01000193);
    return d >> 8;
}

template <typename _CharT>
struct elsa<std::basic_string_view<_CharT>> {
    constexpr std::size_t operator()(std::basic_string_view<_CharT> const& value) const {
        return hashString(value);
    }
    constexpr std::size_t operator()(std::basic_string_view<_CharT> const& value,
                                     std::size_t seed) const {
        return hashString(value, seed);
    }
};

template <typename Key, typename Val, std::size_t N, 
          typename Hash = elsa<Key>, 
          typename KeyEqual = std::equal_to<Key>>
class StaticHashMap {
    inline static constexpr std::size_t StorageSize 
        = nextHighestPowerOfTwo(N);

    using container_type = std::array<std::pair<Key, Val>, N>;
    using tables_type = PmhTables<StorageSize, Hash>;

    KeyEqual const _equal;
    container_type _items;
    tables_type _tables;
public:
    using key_type = Key;
    using mapped_type = Val;
    using value_type = typename container_type::value_type;

    constexpr StaticHashMap(container_type items, Hash const& hash,
                            KeyEqual const& equal) 
        : _equal{equal}
        , _items{items}
        , _tables{makePmhTables<StorageSize>(
            _items, hash, GetKey{}, default_prg_t{114514})} 
    {}

    constexpr StaticHashMap(container_type items) 
        : StaticHashMap{items, Hash{}, KeyEqual{}} 
    {}

    constexpr Val const& at(Key const& key) const {
        auto& kv = lookup(key);
        if (_equal(kv.first, key))
            return kv.second;
        else
            throw std::out_of_range("unknown key");
    }

    constexpr auto const& lookup(Key const& key) const noexcept {
        return _items[_tables.lookup(key)];
    }
};

// test
int main() {
    using namespace std::string_view_literals;
    constexpr StaticHashMap<std::string_view, std::size_t, 4> mp{
        std::array<std::pair<std::string_view, std::size_t>, 4>{
            std::pair<std::string_view, std::size_t>{"1", 1},
            {"2", 1},
            {"3", 1},
            {"4", 1},
        }
    };

    static_assert(mp.at("1") == 1, "111");
}