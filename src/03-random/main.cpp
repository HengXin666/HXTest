#include <random>

#include <HXprint/print.h>

struct XorShift32 {
    uint32_t a;

    explicit XorShift32(size_t seed = 0) 
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

    static constexpr uint32_t min() noexcept {
        return 1;
    }

    static constexpr uint32_t max() noexcept {
        return UINT32_MAX;
    }
};

struct WangSHash {
    uint32_t a;

    explicit WangSHash(size_t seed = 0) 
        : a(static_cast<uint32_t>(seed)) 
    {}

    using result_type = uint32_t;

    constexpr uint32_t operator()() noexcept {
        uint32_t x = a;
        x = (x ^ 61) ^ (x >> 16);
        x *= 9;
        x = x ^ (x >> 4);
        x *= 0x27d4eb2d;
        x = x ^ (x >> 15);
        return a = x;
    }

    static constexpr uint32_t min() noexcept {
        return 0;
    }

    static constexpr uint32_t max() noexcept {
        return UINT32_MAX;
    }
};

int main() {
    auto test1 = [] {
        // 1. 定义随机数引擎生成器 (构造传参是随机数种子, 此处传入基于cpu的真随机数(开销大~))
        std::mt19937 rng{std::random_device{}()};

        // 2. 均匀分布: 产生在范围内均匀分布的整数
        std::uniform_int_distribution<int> uni(1, 10);

        for (int i = 0; i < 10; ++i)
            HX::print::print(uni(rng), ' ');
        HX::print::println("\n");
    };
    test1();

    auto test2 = [] {
        WangSHash rng{std::random_device{}()};
        std::uniform_int_distribution<int> uni(1, 10);

        for (int i = 0; i < 10; ++i)
            HX::print::print(uni(rng), ' ');
        HX::print::println("\n");
    };
    test2();

    auto test3 = [] {
        HX::print::println("XorShift32:");
        for (std::size_t i = 0; i < 10; ++i) {
            XorShift32 rng{i};
            std::uniform_int_distribution<int> uni(1, 100);
            HX::print::print(uni(rng), ' ');
        }

        HX::print::println("\nWangSHash:");
        for (std::size_t i = 0; i < 10; ++i) {
            WangSHash rng{i};
            std::uniform_int_distribution<int> uni(1, 100);
            HX::print::print(uni(rng), ' ');
        }
        HX::print::println("\n");
    };
    test3();

    auto test4 = [] {
        std::vector<float> pArr = {0.6f, 0.25f, 0.05f, 0.1f};
        std::vector<float> pArrScanned;
        // 求`pArr`的元素和, 即 sum += pArr[i]; pArrScanned[i] = sum;
        std::inclusive_scan(pArr.begin(), pArr.end(), std::back_inserter(pArrScanned));
        HX::print::println("pArr: ", pArr);
        HX::print::println("pArrScanned: ", pArrScanned);
        std::vector<std::string> res = {"一星", "二星", "四星", "三星"};
        std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<float> unf(0.0f, 1.0f);
        auto getRes = [&] () -> std::string {
            float f = unf(rng);
            // 二分, 确定概率区间
            auto it = std::lower_bound(pArrScanned.begin(), pArrScanned.end(), f);
            if (it == pArrScanned.end()) [[unlikely]] {
                return "";
            }
            return res[static_cast<std::size_t>(it - pArrScanned.begin())];
        };
        HX::print::println(getRes());
    };
    test4();
    return 0;
}
