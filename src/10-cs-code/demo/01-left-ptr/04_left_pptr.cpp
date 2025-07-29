#include <HXTest.hpp>

struct IsCopy {
    constexpr IsCopy() noexcept = default;
    IsCopy(IsCopy const&) noexcept {
        log::hxLog.error("发生了拷贝构造"); 
    }
    constexpr IsCopy(IsCopy&&) noexcept = default;

    IsCopy& operator=(IsCopy const&) noexcept {
        log::hxLog.error("发生了拷贝赋值");
        return *this;
    }

    IsCopy& operator=(IsCopy&&) noexcept = default;
};

struct LeftPtr {
    IsCopy data_1;
};

struct A {
    int val;
};

namespace {

[[maybe_unused]] inline constexpr struct __val {} val;

auto operator<(__val, uint64_t self) noexcept {
    return reinterpret_cast<A*>(self)->val;
}

}

#define 一 (uint64_t)

HX_NO_WARNINGS_BEGIN
int main() {
    A* ap = new A{2233};
    auto&& resEnd = val <一 ap;
    return 0;
}
HX_NO_WARNINGS_END