#include <HXprint/print.h>

using namespace HX;

// 取消注释以测试 二阶段名称查找
// #define ENABLE_THE_TWO_STAGE_NAME_LOOKUP

template <typename T>
constexpr std::string_view func(T, bool isReturn = false) {
    if (!isReturn) {
#ifndef ENABLE_THE_TWO_STAGE_NAME_LOOKUP
        return func(1, true); // 当前看不到 下面的声明, 所以匹配的是自己 func<int>(...)
#else
        // 注意: 当且仅当 指明了 模板类型 的情况会出问题, 如果是传递 基于模板的类型, 则是使用 二阶段名称查找
        return func<T>({}, true); 
#endif
    }
    if constexpr (std::is_same_v<T, int>) {
        return "T: int";
    } else {
        return "T";
    }
}

#ifdef ENABLE_THE_TWO_STAGE_NAME_LOOKUP
template <typename T>
    requires(std::is_same_v<T, decltype(1.f)>)
constexpr std::string_view func(T, bool = false) {
    return "int"; // 因为测试使用的是 T = float
}
#endif

constexpr std::string_view func(int, bool = false) {
    return "int";
}

// 取消下面的注释, 则切换为 `静态函数`
// #define SET_SWITCH_TO_STATIC_FUNC

#ifdef SET_SWITCH_TO_STATIC_FUNC
#define SWITCH_TO_STATIC_FUNC static
#else
#define SWITCH_TO_STATIC_FUNC
#endif // !SET_SWITCH_TO_STATIC_FUNC

struct Func {
    template <typename T>
    SWITCH_TO_STATIC_FUNC constexpr std::string_view thisFunc(T, bool isReturn = false) {
        if (!isReturn) {
            return thisFunc(1, true); // 因为在类里面, 所以可以知道整个类内的函数声明, 找到更加匹配的版本
        }
        if constexpr (std::is_same_v<T, int>) {
            return "T: int";
        } else {
            return "T";
        }
    }

    SWITCH_TO_STATIC_FUNC constexpr std::string_view thisFunc(int, bool = false) {
        return "int";
    }
};

int main() {
    // 开启二阶段会报错, 以标识它有所不同的匹配
    static_assert(func(1) != func(1.f), "");

#ifdef SET_SWITCH_TO_STATIC_FUNC
    static_assert(Func::thisFunc(1) == Func::thisFunc(1.f), "");
#else
    static_assert(Func{}.thisFunc(1) == Func{}.thisFunc(1.f), "");
#endif // !SET_SWITCH_TO_STATIC_FUNC
    return 0;
}