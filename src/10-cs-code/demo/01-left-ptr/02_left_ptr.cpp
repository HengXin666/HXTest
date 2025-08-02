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
    IsCopy data;
};

LeftPtr& operator-(LeftPtr& self) noexcept {
    return self; 
}

struct __hx_data {};

inline static __hx_data data;

auto operator<(__hx_data, LeftPtr& self) noexcept {
    return self.data;
}

HX_NO_WARNINGS_BEGIN
int main() {
    LeftPtr lp;
    // 如何实现一个左指针运算符? data<-obj
    auto res = data<-lp;
    return 0;
}
HX_NO_WARNINGS_END