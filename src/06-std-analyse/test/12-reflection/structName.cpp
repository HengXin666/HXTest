#include <HXLibs/log/Log.hpp>

template <typename T>
inline constexpr std::string_view getStructName() {
#if defined(_MSC_VER)
    constexpr std::string_view funcName = __FUNCSIG__;
#else
    constexpr std::string_view funcName = __PRETTY_FUNCTION__;
#endif
    
#if defined(__clang__)
    auto split = funcName.substr(funcName.rfind("T = ") + sizeof("T = ") - 1);
    auto pos = split.rfind(']');
    split = split.substr(0, pos);
    pos = split.find("<");
    if (pos != std::string_view::npos) {
        split = split.substr(0, pos);
    }
    pos = split.rfind("::");
    if (pos != std::string_view::npos) {
        split = split.substr(pos + 2);
    }
    return split;
#elif defined(__GNUC__)
    auto split = funcName.substr(funcName.find("[with T = "));
    split = split.substr(split.find("::"));
    auto pos = split.find(';');
    split = split.substr(0, pos);
    pos = split.find("<");
    if (pos != std::string_view::npos) {
        split = split.substr(0, pos);
    }
    pos = split.rfind("::");
    if (pos != std::string_view::npos) {
        split = split.substr(pos + 2);
    }
    return split;
#elif defined(_MSC_VER)
    auto split = funcName.substr(funcName.rfind("getStructName<") + sizeof("getStructName<") - 1);
    auto pos = split.find("<");
    if (pos != std::string_view::npos) {
        split = split.substr(0, pos);
    } else {
        pos = split.find('>');
        split = split.substr(0, pos);
    }
    pos = split.rfind("::");
    if (pos != std::string_view::npos) {
        split = split.substr(pos + 2);
    }
    return split;
#else
    static_assert(
        false, 
        "You are using an unsupported compiler. Please use GCC, Clang "
        "or MSVC or switch to the rfl::Field-syntax."
    );
#endif
}

using namespace HX;

int main() {
    // 测试 https://godbolt.org/z/o1sMYs15j

    struct LoLiInfo {

    };

    log::hxLog.info(getStructName<class A>());
    log::hxLog.info(getStructName<struct B>());
    constexpr auto name = getStructName<LoLiInfo>();
    static_assert(name == "LoLiInfo", "");
    constexpr auto lambda = getStructName<decltype([]{})>();
    log::hxLog.warning(lambda);
    return 0;
}