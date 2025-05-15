#include <HXprint/print.h>

/**
 * @brief 本文件实现的是 trivially_copyable 类型 的tuple
 * 也就是说, 其内部不能有自定义的析构、移动赋值、拷贝赋值、虚函数、虚基类
 * 不然就不是 可拷贝的平凡类型
 *
 * 学习自 http://www.purecpp.cn/detail?id=2309
 */

template <typename T, std::size_t Idx>
struct TupleData {
    using type = T;
    T _data;
};

template <typename Idx, typename... Ts>
struct TupleImpl;

template <typename... Ts, std::size_t... Idx>
struct TupleImpl<std::index_sequence<Idx...>, Ts...> 
    : public TupleData<Ts, Idx>... 
{
    TupleImpl() = default;
    TupleImpl(Ts&&... ts)
        : TupleData<Ts, Idx>{std::forward<Ts>(ts)}...
    {}
};

template <typename... Ts>
struct Tuple : public TupleImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
    // 确保是 is_trivially_copyable_v
    static_assert((std::is_trivially_copyable_v<Ts> && ...),
                  "All types in Tuple must be trivially copyable");
    using base = TupleImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;
    using base::base;
};

template <typename T, std::size_t Idx>
decltype(auto) get(const TupleData<T, Idx>& tp) noexcept {
    return tp._data;
}

template <typename T, std::size_t Idx>
decltype(auto) get(TupleData<T, Idx>& tp) noexcept {
    return tp._data;
}

int main() {
    Tuple<int, double> tp{1, 5.14};
    auto tp2 = tp;

    (void)tp2;
    HX::print::println(get<int, 0>(tp));
    return 0;
}