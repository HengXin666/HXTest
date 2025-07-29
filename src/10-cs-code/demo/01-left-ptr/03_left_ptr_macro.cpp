#include <HXTest.hpp>
#include <HXLibs/macro/join.hpp>
#include <HXLibs/macro/for.hpp>

#define __HX_MACRO_FOR_IMPL_THIS_2__() __HX_MACRO_FOR_IMPL_2__
#define __HX_MACRO_FOR_IMPL_2__0(macro, k, x, ...) macro(k, x) HX_DELAY(__HX_MACRO_FOR_IMPL_THIS_2__)()(macro, k, __VA_ARGS__)
#define __HX_MACRO_FOR_IMPL_2__1(...)
#define __HX_MACRO_FOR_IMPL_2__(macro, k, x, ...) HX_JOIN(__HX_MACRO_FOR_IMPL_2__, IF_EMPTY(x))(macro, k, x, __VA_ARGS__)

/**
 * @brief for展开宏, 
 * @param macro 宏函数, 应该接受一个变量
 * @param x... 宏参数, 它们会被依次传入 macro 函数
 */
#define HX_FOR_2(macro, k, x, ...) HX_EVAL(__HX_MACRO_FOR_IMPL_2__(macro, k, x, __VA_ARGS__))

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

#define REFLLEFT_ARROW_NAME(__CLASS_TYPE__, __NAME__)                          \
    namespace __my_metadata__ {                                                \
    struct HX_JOIN(HX_JOIN(__hx_my_, __LINE__), __NAME__) {};                  \
    }                                                                          \
                                                                               \
    namespace {                                                                \
    [[maybe_unused]] inline constexpr static __my_metadata__::HX_JOIN(         \
        HX_JOIN(__hx_my_, __LINE__), __NAME__) __NAME__;                       \
    }                                                                          \
                                                                               \
    constexpr auto& operator<(                                                 \
        __my_metadata__::HX_JOIN(HX_JOIN(__hx_my_, __LINE__), __NAME__)        \
            const&,                                                            \
        __CLASS_TYPE__ const& HX_JOIN(__hx_my_self_, __LINE__)) noexcept {     \
        return HX_JOIN(__hx_my_self_, __LINE__).__NAME__;                      \
    }                                                                          \
    constexpr auto& operator<(                                                 \
        __my_metadata__::HX_JOIN(HX_JOIN(__hx_my_, __LINE__), __NAME__)        \
            const&,                                                            \
        __CLASS_TYPE__& HX_JOIN(__hx_my_self_, __LINE__)) noexcept {           \
        return HX_JOIN(__hx_my_self_, __LINE__).__NAME__;                      \
    }

#define REFLLEFT_ARROW(__CLASS_TYPE__, ...)                                    \
    constexpr auto& operator-(__CLASS_TYPE__ const& self) noexcept {           \
        return self;                                                           \
    }                                                                          \
    constexpr auto& operator-(__CLASS_TYPE__& self) noexcept {                 \
        return self;                                                           \
    }                                                                          \
    HX_FOR_2(REFLLEFT_ARROW_NAME, __CLASS_TYPE__, __VA_ARGS__)

struct LeftPtr {
    IsCopy data_1;
    LeftPtr* self = nullptr;
};

// 注册
REFLLEFT_ARROW(LeftPtr, data_1, self)

HX_NO_WARNINGS_BEGIN
int main() {
    LeftPtr lp;
    auto& res = data_1<-*(self<-(*(self<-lp)));
    return 0;
}
HX_NO_WARNINGS_END