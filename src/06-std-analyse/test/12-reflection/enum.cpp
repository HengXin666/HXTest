#include <HXTest.hpp>

enum class EnumClass {
    A = 0,
    B = 2,
    C = 4
};

namespace HX {

template <typename T, T... Is>
struct IntegerIndex {
    inline static constexpr T val[] {Is...};
};

template <typename T, T Now, T End, T Stride, T... Is>
constexpr auto makeIntegerIndexByRange() {
    if constexpr (Now + Stride < End) {
        return makeIntegerIndexByRange<T, Now + Stride, End, Stride, Is..., Now>();
    } else {
        return IntegerIndex<T, Is..., Now>{};
    }
}

template <typename T, T Begin, T End, T Stride = 1>
using makeIntegerIndexRange = decltype(makeIntegerIndexByRange<T, Begin, End, Stride>());

/**
 * @brief 获取枚举或者枚举类的值名称
 * @tparam EnumType 
 * @tparam Enum 
 * @return constexpr std::string_view 
 */
template <typename EnumType, EnumType Enum>
inline constexpr std::string_view getEnumName() {
#if defined(_MSC_VER)
    constexpr std::string_view funcName = __FUNCSIG__;
#else
    constexpr std::string_view funcName = __PRETTY_FUNCTION__;
#endif
#if defined(__clang__)
    auto split = funcName.substr(funcName.rfind("Enum = "));
    split = split.substr(7, split.size() - 8);
#elif defined(__GNUC__)
    auto split = funcName.substr(funcName.rfind("Enum = "));
    split = split.substr(7, split.find("; ") - 7);
#elif defined(_MSC_VER)
    auto split = funcName.substr(funcName.rfind(","));
    split = split.substr(1, split.find(">(void)") - 1);
#else
    static_assert(
        false, 
        "You are using an unsupported compiler. Please use GCC, Clang "
        "or MSVC or switch to the rfl::Field-syntax."
    );
#endif
    [&] {
        // 如果是 enum class, 那么会有 Type::name
        auto pos = split.find("::");
        if (pos == std::string_view::npos) {
            return;
        }
        split = split.substr(pos + 2);
    }();
    return split;
}

template <typename T>
constexpr std::string_view findEnumName(T const& enumVal) {
    return [&] <int... Idx> (IntegerIndex<int, Idx...>) {
        std::string_view res{};
        ([&]{
            if (Idx == static_cast<int>(enumVal)) {
                res = getEnumName<T, static_cast<T>(Idx)>();
                return true;
            }
            return false;
        }() || ...);
        return res;
    }(makeIntegerIndexRange<int, 0, 5>{});
}

template <typename T>
constexpr T toEnum(std::string_view name) {
    return [&] <int... Idx> (IntegerIndex<int, Idx...>) {
        T res;
        bool isFind = false;
        ([&]{
            if (name == getEnumName<T, static_cast<T>(Idx)>()) {
                res = static_cast<T>(Idx);
                isFind = true;
                return true;
            };
            return false;
        }() || ...);
        if (!isFind) [[unlikely]] {
            // 找不到对应枚举名称
            throw std::runtime_error{"not find enum name"};
        }
        return res;
    }(makeIntegerIndexRange<int, 0, 5>{});
}

} // namespace HX

using namespace HX;

// 在线测试: https://godbolt.org/z/7o4eoWvf3
int main() {
    EnumClass a = static_cast<EnumClass>(0);
    log::hxLog.info(findEnumName(a));
    auto toA = toEnum<EnumClass>("A");
    if (a == toA) {
        log::hxLog.debug("相等的!");
    } else {
        log::hxLog.error("大错特错!");
    }
    log::hxLog.info(findEnumName(toA));
    return 0;
}