#include <HXprint/print.h>

template <size_t Idx, typename... Ts>
struct _tuple;

template <size_t Idx>
struct _tuple<Idx> {

};

template <typename T>
struct _tuple_head_base {
    T t;
};

template <size_t Idx, typename T, typename... Ts>
struct _tuple<Idx, T, Ts...> 
    : public _tuple<Idx + 1, Ts...>
    , private _tuple_head_base<T> 
{
    template <size_t N, typename...>
    friend struct _tuple;

    using _head = _tuple_head_base<T>;
    using _tail = _tuple<Idx + 1, Ts...>;

    explicit _tuple(T&& t, Ts&&... ts)
        : _tuple<Idx + 1, Ts...>(std::forward<Ts>(ts)...)
        , _tuple_head_base<T>(std::forward<T>(t))
    {}

    constexpr const T& _get_head() const noexcept {
        return _head::t;
    }

    constexpr T& _get_head() noexcept {
        return _head::t;
    }

    // 引用多态
    constexpr const _tail& _get_tail(const _tuple& t) const noexcept {
        return t;
    }

    constexpr _tail& _get_tail(_tuple& t) noexcept {
        return t;
    }
};

template <typename... Ts>
struct tuple : public _tuple<0, Ts...> {
    explicit tuple(Ts&&... ts)
        : _tuple<0, Ts...>(std::forward<Ts>(ts)...)
    {}
};

/**
 * @brief get 部分, 需要准备3个版本(因为无法使用 T&& + std::forward)
 */

template <size_t N, std::size_t Idx = 0, typename... Ts>
inline constexpr auto& _get(_tuple<Idx, Ts...>& tp) {
    if constexpr (N == Idx) {
        return tp._get_head();
    } else {
        return _get<N, Idx + 1>(tp._get_tail(tp));
    }
}

template <size_t N, std::size_t Idx = 0, typename... Ts>
inline constexpr const auto& _get(const _tuple<Idx, Ts...>& tp) {
    if constexpr (N == Idx) {
        return tp._get_head();
    } else {
        return _get<N, Idx + 1>(tp._get_tail(tp));
    }
}

template <size_t N, std::size_t Idx = 0, typename... Ts>
inline constexpr auto&& _get(_tuple<Idx, Ts...>&& tp) {
    if constexpr (N == Idx) {
        return std::move(tp._get_head());
    } else {
        return _get<N, Idx + 1>(std::move(tp._get_tail(tp)));
    }
}

template <size_t Idx, typename... Ts, typename = std::enable_if_t<(Idx < sizeof...(Ts))>>
inline constexpr auto& get(tuple<Ts...>& tp) {
    return _get<Idx>(tp);
}

template <size_t Idx, typename... Ts, typename = std::enable_if_t<(Idx < sizeof...(Ts))>>
inline constexpr const auto& get(const tuple<Ts...>& tp) {
    return _get<Idx>(tp);
}

template <size_t Idx, typename... Ts, typename = std::enable_if_t<(Idx < sizeof...(Ts))>>
inline constexpr auto&& get(tuple<Ts...>&& tp) {
    return std::move(_get<Idx>(std::move(tp)));
}

int main() {
    tuple<int, double, tuple<std::string, int>> t{1, 3.14, tuple<std::string, int>{"awa", 1}};
    const auto& tv = t;
    auto& res = get<0>(get<2>(tv));
    HX::print::println("get<1>: ", res);
    return 0;
}