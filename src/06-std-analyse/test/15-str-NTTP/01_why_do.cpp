#include <HXTest.hpp>

namespace HX {

constexpr std::chrono::seconds operator""_s(long double) {
    return {};
}

template <char... Cs>
constexpr std::chrono::seconds operator""_s() {
    static_assert(((Cs >= '0' && Cs <= '9'), ...), "");
    static_assert((('0' <= Cs && Cs <= '9') && ...) && sizeof...(Cs) <= 18,
        "Only numeric characters allowed");
    constexpr char str[] = {Cs...};
    std::size_t value = 0;
    for (std::size_t i = 0; i < sizeof...(Cs); ++i) {
        value = value * static_cast<std::size_t>(10) 
              + static_cast<std::size_t>(str[i] - '0');
    }
    return std::chrono::seconds{value};
}

} // namespace HX

HX_NO_WARNINGS_BEGIN
void test() {
    constexpr auto time1 = std::chrono::operator""s(12);
    constexpr auto time2 = std::chrono::operator""s<'1', '2'>();
    static_assert(time1 == time2, "");
    // constexpr auto time3 = std::chrono::operator""s<"123">();
}

int main() {
    using namespace std::chrono;
    auto time = 1s;
    // using StrType = decltype("123");
    // auto str = "123";
    return 0;
}
HX_NO_WARNINGS_END