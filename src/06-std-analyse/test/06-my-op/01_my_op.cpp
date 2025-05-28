#include <HXprint/print.h>

/**
 * @brief 自定义全局运算符重载
 */
uint64_t operator|(uint64_t a, std::string const& s) {
    return a + s.size();
}

/**
 * @brief 自定义字面量
 * @param str 
 * @param len 
 * @return std::string 
 */
std::string operator""_hx(const char* str, std::size_t len) {
    return {str, len};
}

std::string operator""_loli(uint64_t x) {
    return std::to_string(x);
}

#if 0
struct A {
    std::string _a;
};

struct B {

};

// Parameter of literal operator must have type 'unsigned long long', 'long double',
// 'char', 'wchar_t', 'char16_t', 'char32_t', or 'const char [ERROR]
B operator""_makeA(A&& a) { // 不支持从自定义类声明字面量
    return {};   
}
#endif

int main() {
    HX::print::println("1 + \"23.33\" == 6 == ", 1 | "23.33"_hx);
    HX::print::println("数字字面量: ", 2233_loli);
    // static constexpr int x = 2233;
    // x_loli <-- 没有这种写法, 必须只能是 字面量, 编译期常量都不行!
    return 0;
}