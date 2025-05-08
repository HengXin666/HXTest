#include <HXprint/print.h>

// 是否支持格式化为 C 风格字符串 (char数组)
// #define _C_TO_STRING_

template <char... Char>
class StringNum {
    inline static constexpr char data[] = {Char...
#ifdef _C_TO_STRING_
        , '\0'
#endif
    };
    inline static constexpr std::size_t len = sizeof...(Char);
public:
    constexpr operator std::string_view() const {
        return {data, len};
    }

    constexpr operator std::string() const {
        return {data, len};
    }
};

struct ToString {
    template <int64_t Num>
    static constexpr auto toString() {
        return _toString<Num >= 0 ? Num : -Num, Num < 0>();
    }

    template <uint64_t Num, bool IsMinus>
    static constexpr auto toString() {
        return _toString<Num, IsMinus>();
    }

private:
    template <uint64_t Num, bool IsMinus, char... Char>
    static constexpr auto _toString() {
        if constexpr (Num < 10) {
            if constexpr (IsMinus) {
                return StringNum<'-', Num + '0', Char...>{};
            } else {
                return StringNum<Num + '0', Char...>{};
            }
        } else {
            return _toString<Num / 10, IsMinus, Num % 10 + '0', Char...>();
        }
    }
};

// 浮点数得 Ryu 算法, 基于 IEEE754 拆位, 这里就不搞了...
// template <double x, typename T>
// constexpr auto fun() {
//     return __FUNCSIG__;
// }

int main() {
    using namespace std::string_view_literals;
    HX::print::println((std::string_view)ToString::toString<0xFFFFFFFFFFFFFFFFULL, false>());
    HX::print::println((std::string_view)ToString::toString<0x7FFFFFFFFFFFFFFFLL>());
    HX::print::println((std::string_view)ToString::toString<1234567>());
    HX::print::println((std::string_view)ToString::toString<1234567ULL>());
    HX::print::println((std::string_view)ToString::toString<1234567LL>());
    HX::print::println((std::string_view)ToString::toString<-1234567>());
    HX::print::println((std::string_view)ToString::toString<-0>());
    HX::print::println(ToString::toString<282000500>() == "282000500"sv);
    HX::print::println(ToString::toString<-282000500>() == "-282000500"sv);
    return 0;
}