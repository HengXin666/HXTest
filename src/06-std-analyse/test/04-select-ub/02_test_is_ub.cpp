#include <HXTest.hpp>

#include <cstdint>
#include <vector>

struct A {
    int a;
    mutable std::vector<int> b;
};

template <typename T>
struct G {
    inline static T val = T{};
};

template <typename T>
inline constexpr auto& getRef() noexcept { return G<T>::val; }

constexpr auto getTp() noexcept {
    auto& [_1, _2] = getRef<A>();
    auto t = std::tie(_1, _2);
    auto makePtr = [](auto&&... args) {
        return std::make_tuple(&args...);
    };
    auto res = std::apply(makePtr, t);
    return res;
}

int ggg;

constexpr auto getGgg() noexcept {
    auto& _1 = ggg;
    return &_1;
}

int main() {
    // 引用的取地址 等于 原本元素的取地址
    auto res = getTp();
    log::hxLog.warning(res);
    auto& [_1, _2] = getRef<A>();
    auto t = std::tie(_1, _2);
    auto makePtr = [](auto&&... args) {
        return std::make_tuple(&args...);
    };
    log::hxLog.warning(std::apply(makePtr, t));
    std::get<1>(res)->push_back(23);

    if (&ggg == getGgg())
        log::hxLog.warning("???");
    return 0;
}