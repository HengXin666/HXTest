#include <HXprint/print.h>

namespace HX {

template <typename T>
struct IsVoid {
    inline static constexpr bool val = false;
};

template <>
struct IsVoid<void> {
    inline static constexpr bool val = true;
};

/**
 * @brief 这个类型是否是 void
 * @tparam T 
 */
template <typename T>
inline constexpr bool IsVoid_V = IsVoid<T>::val;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-==-=-=-=-==-=-=-=-=

template <typename T>
struct RemoveReference {
    using type = T;
};

template <typename T>
struct RemoveReference<T&> {
    using type = T;
};

/**
 * @brief 去掉类型引用
 * @tparam T 
 */
template <typename T>
using RemoveReference_T = RemoveReference<T>::type;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-==-=-=-=-==-=-=-=-=

template <typename T, typename U>
struct IsSame {
    inline static constexpr bool val = false;
};

template <typename T>
struct IsSame<T, T> {
    inline static constexpr bool val = true;
};

/**
 * @brief 判断这两个类型是否完全一样
 * @tparam T 
 * @tparam U 
 */
template <typename T, typename U>
inline constexpr bool IsSame_V = IsSame<T, U>::val;

} // namespace HX

#define INFO_CODE(__CODE__) HX::print::println(#__CODE__": ", __CODE__)

int main() {
    using namespace HX;

    INFO_CODE(IsVoid_V<int>);  // false
    INFO_CODE(IsVoid_V<void>); // true

    using T1 = RemoveReference_T<int>;  // int
    using T2 = RemoveReference_T<int&>; // int

    INFO_CODE((IsSame_V<int, int&>)); // false
    INFO_CODE((IsSame_V<T1, T2>));    // true
    return 0;
}

#undef INFO_CODE