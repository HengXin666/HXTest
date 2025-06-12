#include <HXprint/print.h>

template <typename Lambda>
struct LambdaData {
    Lambda& operator()() const noexcept {
        return lamda;
    }
    Lambda& lamda;
};

template <std::size_t N, typename Lambda,
    typename Res = decltype(std::declval<Lambda>()(
        (std::declval<double>())))>
Res fun01(int index, Lambda&& lambda) {
    return [&] <std::size_t... Idx>(std::index_sequence<Idx...>) -> Res {
        using FuncPtr = Res (*)(Lambda&);
        static FuncPtr table[] {
            [] (Lambda& ld) -> Res {
                (void)Idx;
                if constexpr (Idx == 0) {
                    return ld(0.0);
                } else if constexpr (Idx == 1) {
                    return ld(std::string{});
                }
            }...
        };
        return table[index](lambda);
    }(std::make_index_sequence<N>{});
}

int main() {

    // auto参数实例化了，函数也是不同对象了
    // 所以上面不能使用 using FuncPtr = Res (*)(Lambda&);
    // 表示相同类型...

    fun01<2>(1, [](auto&&) {
        return [] {
            HX::print::println("const Ts &ts...");
        };
    })();

    return 0;
}