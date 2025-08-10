#include <HXTest.hpp>
#include <HXLibs/macro/join.hpp>
#include <HXLibs/macro/for.hpp>

#include <vector>
#include <functional>
#include <any>

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
    [[maybe_unused]] inline constexpr __my_metadata__::HX_JOIN(                \
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

    auto func1(int x, std::vector<int>& arr) {
        int sum = 0;
        for (auto& v : arr)
            sum += (v += x);
        return sum;
    }

    auto func2(int x, std::vector<int>& arr) {
        return func1(x, arr);
    }
};

// 注册
REFLLEFT_ARROW(LeftPtr, data_1, self)

//////////////////////////////////////////////////////////////////////////////////////

namespace {

struct __hx__func1 {
    template <typename... Args>
    auto& operator()(Args&&... args) {
        _f = [..._args = std::forward<Args>(args)](LeftPtr& self) mutable {
            if constexpr (!requires {
                self.func1(_args...);
            }) {
                static_assert(sizeof...(Args) < 0, "sb");
            }
            return self.func1(_args...);
        };
        return *this;
    }

    std::function<std::any(LeftPtr&)> _f;
};

[[maybe_unused]] inline static __hx__func1 func1;

}

[[maybe_unused]] decltype(auto) operator<(__hx__func1 _func, LeftPtr& self) {
    return _func._f(self);
}

//////////////////////////////////////////////////////////////////////////////////////

namespace {

enum class LeftPtrType {};

template <typename Lambda>
struct SelfLambda {
    using Type = LeftPtrType;
    Lambda _cb;
};

template <typename... Args>
inline auto func2(Args&&... args) {
    SelfLambda _resCb{[..._args = std::forward<Args>(args)](LeftPtr& self) mutable {
        if constexpr (!requires {
            self.func2(_args...);
        }) {
            static_assert(sizeof...(Args) < 0, "sb");
        }
        return self.func2(_args...);
    }};
    return _resCb;
}

}

template <typename T>
    requires(std::is_same_v<LeftPtrType, typename T::Type>)
[[maybe_unused]] decltype(auto) operator<(T&& _func2, LeftPtr& self) {
    return _func2._cb(self);
}

//////////////////////////////////////////////////////////////////////////////////////

HX_NO_WARNINGS_BEGIN
int main() {
    std::vector<int> arr{1, 2, 3, 4 ,5};
    // auto& res = data_1<-*(self<-(*(self<-lp))); // ub: *nullptr
    
    LeftPtr obj;
    auto res = data_1<-obj;
    auto funcRes01 = func1(1, arr)<-obj;
    auto funcRes02 = func2(1, arr)<-obj;

    if (std::any_cast<int>(funcRes01) == funcRes02) {
        log::hxLog.info("对的对的");
        log::hxLog.warning("res: ", funcRes02);
    } else {
        log::hxLog.debug(arr);
        log::hxLog.error("大错特错", std::any_cast<int>(funcRes01), funcRes02);
    }

    return 0;
}
HX_NO_WARNINGS_END