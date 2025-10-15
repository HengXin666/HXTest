#include <HXTest.hpp>

namespace HX {

template <char... Cs>
struct CStrWrap {};

template <std::size_t N>
struct CStr {
    char _str[N];

    constexpr CStr(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            _str[i] = str[i];
    }

    constexpr char operator[](std::size_t i) const noexcept {
        return _str[i];
    }

    constexpr auto size() const noexcept {
        return N - 1; // 去掉 '\0'
    }
};

template <CStr Str>
using ToCs = decltype(
    [] <std::size_t... Idx> (std::index_sequence<Idx...>) {
        return CStrWrap<Str[Idx]...>{};
    }(std::make_index_sequence<Str.size()>{})
);

template <char... Cs>
constexpr auto toNum(CStrWrap<Cs...>) {
    static_assert((('0' <= Cs && Cs <= '9') && ...) && sizeof...(Cs) <= 18,
        "Only numeric characters allowed");
    constexpr char str[] = {Cs...};
    std::size_t value = 0;
    for (std::size_t i = 0; i < sizeof...(Cs); ++i) {
        value = value * static_cast<std::size_t>(10) 
              + static_cast<std::size_t>(str[i] - '0');
    }
    return value;
}

template <CStr Str>
constexpr std::chrono::seconds operator""_s() {
    return std::chrono::seconds{toNum(ToCs<Str>{})};
}

template <char... Cs>
void strTp() {}

template <auto Str>
void strAutoTp() {}

template <CStr Str>
void strCStrTp() {}

} // namespace HX

HX_NO_WARNINGS_BEGIN
void test() {
    constexpr auto time1 = std::chrono::operator""s(12);
    constexpr auto time2 = std::chrono::operator""s<'1', '2'>();
    static_assert(time1 == time2, "");
    // constexpr auto time3 = std::chrono::operator""s<"123">();

    // strTp<"123">();
    // strAutoTp<"123">();
    // strCStrTp<"123">();
}

int main() {
    using namespace std::chrono;
    constexpr auto time1 = 123s;
    // using StrType = decltype("123");
    // auto str = "123";

    constexpr auto time2 = HX::operator""_s<"123">();
    static_assert(time1 == time2, "");
    return 0;
}
HX_NO_WARNINGS_END