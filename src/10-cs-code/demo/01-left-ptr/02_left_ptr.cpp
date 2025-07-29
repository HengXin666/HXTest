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

HX_NO_WARNINGS_BEGIN
int main() {
    LeftPtr lp;
    auto data_1 = &LeftPtr::data_1;
    return 0;
}
HX_NO_WARNINGS_END