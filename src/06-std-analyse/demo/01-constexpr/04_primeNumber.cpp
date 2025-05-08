#include <HXprint/print.h>

#include <vector>

template <uint64_t... Nums>
struct ResPrime {
    inline static constexpr uint64_t data[] = {Nums...};
    inline static constexpr std::size_t len = sizeof...(Nums);

    template <typename NumType>
    constexpr operator std::vector<NumType>() const {
        return {data, data + len};
    }
};

struct ifConstexprPrime {
    template <uint64_t Num>
    static constexpr bool isPrime() {
        if constexpr (Num == 1) {
            return false;
        } else {
            return _isPrime<Num, 2>();
        }
    }
    
    template <uint64_t Num, uint64_t Idx>
    static constexpr bool _isPrime() {
        if constexpr (Idx * Idx > Num) {
            return true;
        } else if constexpr (Num % Idx) {
            return _isPrime<Num, Idx + 1>();
        } else {
            return false;
        }
    }
};

template <uint64_t Idx, uint64_t N, uint64_t... Nums>
constexpr auto _getPrimeArray() {
    if constexpr (Idx != N) {
        if constexpr (ifConstexprPrime::isPrime<Idx>()) {
            return _getPrimeArray<Idx + 1, N, Nums..., Idx>();
        } else {
            return _getPrimeArray<Idx + 1, N, Nums...>();
        }
    } else {
        return ResPrime<Nums...>{};
    }
}

template <uint64_t N>
constexpr auto getPrimeArray() {
    return _getPrimeArray<2, N>();
}

int main() {
    HX::print::println(ifConstexprPrime::isPrime<1>());
    HX::print::println(ifConstexprPrime::isPrime<2>());
    HX::print::println(ifConstexprPrime::isPrime<3>());
    HX::print::println(ifConstexprPrime::isPrime<4>());
    HX::print::println(ifConstexprPrime::isPrime<5>());
    HX::print::println(ifConstexprPrime::isPrime<10>());
    HX::print::println((std::vector<uint64_t>)getPrimeArray<128>());
    return 0;
}