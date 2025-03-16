#include <HXprint/print.h>

template <size_t Idx, typename... Ts>
struct _tuple;

template <size_t Idx>
struct _tuple<Idx> {};

template <size_t Idx, typename T>
struct _tuple_head_base {
    T t;

    template <typename U = T>
    constexpr _tuple_head_base(U&& _t) noexcept
        : t(std::forward<U>(_t))
    {}

    static constexpr const T& _get_t(const _tuple_head_base& t) noexcept {
        return t.t;
    }

    static constexpr T& _get_t(_tuple_head_base& t) noexcept {
        return t.t;
    }
};

template <size_t Idx, typename T, typename... Ts>
struct _tuple<Idx, T, Ts...> 
    : public _tuple<Idx + 1, Ts...>
    , private _tuple_head_base<Idx, T> 
{
    using _head = _tuple_head_base<Idx, T>;
    using _tail = _tuple<Idx + 1, Ts...>;

    constexpr explicit _tuple() noexcept
        : _tuple<Idx + 1, Ts...>()
        , _tuple_head_base<Idx, T>()
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr explicit _tuple(U&& t, Us&&... ts) noexcept
        : _tuple<Idx + 1, Ts...>(std::forward<Us>(ts)...)
        , _tuple_head_base<Idx, T>(std::forward<U>(t))
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr explicit _tuple(_tuple<Idx, U, Us...>& t) noexcept
        : _tuple<Idx + 1, Ts...>(_tuple<Idx, U, Us...>::_get_tail(t))
        , _tuple_head_base<Idx, T>(_tuple<Idx, U, Us...>::_get_head(t))
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us)
            && !std::is_same_v<_tuple<Idx, T, Ts...>, _tuple<Idx, U, Us...>>)>>
    constexpr explicit _tuple(const _tuple<Idx, U, Us...>& t) noexcept
        : _tuple<Idx + 1, Ts...>(_tuple<Idx, U, Us...>::_get_tail(t))
        , _tuple_head_base<Idx, T>(_tuple<Idx, U, Us...>::_get_head(t))
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr explicit _tuple(_tuple<Idx, U, Us...>&& t) noexcept
        : _tuple<Idx + 1, Ts...>(std::move(_tuple<Idx, U, Us...>::_get_tail(t)))
        , _tuple_head_base<Idx, T>(std::move(_tuple<Idx, U, Us...>::_get_head(t)))
    {}

    constexpr _tuple(const _tuple&) = default;
    constexpr _tuple(_tuple&&) = default;

    constexpr _tuple& operator=(const _tuple&) = delete; // 禁止拷贝赋值
    constexpr _tuple& operator=(_tuple&&) = default;

    static constexpr const T& _get_head(const _tuple& t) noexcept {
        return _head::_get_t(t);
    }

    static constexpr T& _get_head(_tuple& t) noexcept {
        return _head::_get_t(t);
    }

    // 引用多态
    static constexpr const _tail& _get_tail(const _tuple& t) noexcept {
        return t;
    }

    static constexpr _tail& _get_tail(_tuple& t) noexcept {
        return t;
    }

    // assign 赋值
    // template <typename U, typename... Us, typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    // constexpr void _assign(const _tuple<Idx, U, Us...>& that) noexcept {
    //     HX::print::println(sizeof...(Us), " ", Idx);
    //     _get_head(*this) = _tuple<Idx, U, Us...>::_get_head(that);
    //     if constexpr (sizeof...(Us) > 0) {
    //         _tuple<Idx + 1, Ts...>::_assign(_tuple<Idx, U, Us...>::_get_tail(that));
    //     }
    // }

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr void _assign(_tuple<Idx, U, Us...>&& that) noexcept {
        _get_head(*this) = std::move(_tuple<Idx, U, Us...>::_get_head(that));
        if constexpr (sizeof...(Us) > 0) {
            _tuple<Idx + 1, Ts...>::_assign(std::move(_tuple<Idx, U, Us...>::_get_tail(that)));
        }
    }
};

template <typename... Ts>
struct tuple : public _tuple<0, Ts...> {
    constexpr tuple()
        : _tuple<0, Ts...>()
    {}

    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us)
            && std::conjunction_v<std::is_convertible<Us, Ts>...>)>> // 允许隐式转换, 方可使用
    constexpr tuple(Us&&... ts)
        : _tuple<0, Ts...>(std::forward<Us>(ts)...)
    {}

    // 为了满足类型转换(可强转) int -> std::size_t
    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us)
        && !std::is_same_v<tuple<Ts...>, tuple<Us...>>)>> // 如果完全相等, 请走默认生成的构造!
    constexpr tuple(tuple<Us...>& t)
        : _tuple<0, Ts...>(t)
    {}

    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us)
            && !std::is_same_v<tuple<Ts...>, tuple<Us...>>)>>
    constexpr tuple(const tuple<Us...>& t)
        : _tuple<0, Ts...>(t)
    {}

    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr tuple(tuple<Us...>&& t)
        : _tuple<0, Ts...>(std::move(t))
    {}

    constexpr tuple(const tuple&) = default;
    constexpr tuple(tuple&&) = default;

    constexpr tuple& operator=(const tuple&) = delete; // 禁止拷贝复制
    constexpr tuple& operator=(tuple&&) = default;

    // template <typename... Us, typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    // constexpr tuple& operator=(const tuple<Us...>& that) noexcept {
    //     this->template _assign<Us...>(that);
    //     return *this;
    // }

    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr tuple& operator=(tuple<Us...>&& that) noexcept {
        this->_assign(std::move(that));
        return *this;
    }
};

/**
 * @brief get 部分, 需要准备3个版本(因为无法使用 T&& + std::forward)
 */
// === get<>(tuple) === {

template <size_t N, std::size_t Idx = 0, typename... Ts>
inline constexpr auto& _get(_tuple<Idx, Ts...>& tp) {
    if constexpr (N == Idx) {
        return tp._get_head(tp);
    } else {
        return _get<N, Idx + 1>(tp._get_tail(tp));
    }
}

template <size_t N, std::size_t Idx = 0, typename... Ts>
inline constexpr const auto& _get(const _tuple<Idx, Ts...>& tp) {
    if constexpr (N == Idx) {
        return tp._get_head(tp);
    } else {
        return _get<N, Idx + 1>(tp._get_tail(tp));
    }
}

template <size_t N, std::size_t Idx = 0, typename... Ts>
inline constexpr auto&& _get(_tuple<Idx, Ts...>&& tp) {
    if constexpr (N == Idx) {
        return std::move(tp._get_head(tp));
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

// } === get<>(tuple) ===

// === std::tuple_size_v<typename Tp> === {

template <typename Tp>
struct _cnt_tuple_v;

template <typename... Ts>
struct _cnt_tuple_v<tuple<Ts...>> {
    constexpr static std::size_t _cnt_v() noexcept {
        return sizeof...(Ts);
    }
};

template <typename T>
constexpr std::size_t cnt_tuple_v = _cnt_tuple_v<std::decay_t<T>>::_cnt_v();

// } === std::tuple_size_v<typename Tp> ===

// === std::make_tuple === {

template <typename... Ts>
constexpr tuple<Ts...> make_tuple(Ts&&... ts) noexcept {
    return tuple<Ts...>{std::forward<Ts>(ts)...};
}

// } === std::make_tuple ===

// === std::tie === {

template <typename... Ts>
constexpr tuple<Ts&...> tie(Ts&... ts) noexcept {
    return tuple<Ts&...>{ts...};
}

// } === std::tie ===

// 实验的: 对于指定了 T = int 类型, 那么优先使用 T 类型, 因此在支持万能引用的T&&作为构造的时候,
// 就无法继续作为万能引用, 而是变为了 int&& (右值引用) 了
// 因此, 可以整个构造函数模版, 让它分别推导
template <typename T>
struct Test {
    template <typename U = T>
    constexpr Test(U&& _t)
        : t(std::forward<U>(_t)) // 这里 forward 用 U 以保持值类别
    {}

    template <typename U>
    constexpr Test<T>& operator=(const Test<U>& that) noexcept {
        t = that.t;
        return *this;
    }

    T t;
};

int main() {
    {
        int a = 1;
        Test<int> t1 (1);
        Test<int&> t2 (a);
        t1 = t2;
    } {
        int a = 1;
        Test<int&> t1 (a);
        Test<int> t2 (1);
        t1 = t2;
    }

    tuple<int, double, tuple<std::string, int>> t{1, 3.14, tuple<std::string, int>{"awa", 1}};
    const auto& tv = t;
    auto& res = get<0>(get<2>(tv));
    HX::print::println("get<1>: ", res, ", tv cnt = ", cnt_tuple_v<decltype(tv)>);

    // 可以跨类型, 如本身是 tuple<const char(&)[N]>, 也可构造为 tuple<std::string>
    tuple<size_t, std::string, float> mt = make_tuple(1, "2", 3.14);
    HX::print::println("get<1>: ", get<1>(mt));

    // tie
    int a = 0, b = 0, c = 0;
    tie(a, b, c) = make_tuple(1, 2, 3);
    HX::print::println("auto [a, b, c] = ", a, " ", b, " ", c);

    {
        int a = 123;
        tuple<int> t1{int{}};
        tuple<int> t2{3};
        tuple<int> t3{a};
        tuple<int> t4(t3);
        t1 = std::move(t2);

        // 拷贝复制 = 拷贝构造 + 移动复制
        // 为什么不行?!
        // t2 = t3;

        (void)t4;
    }
    return 0;
}