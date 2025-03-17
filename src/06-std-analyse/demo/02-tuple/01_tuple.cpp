#include <HXprint/print.h>

// 删除拷贝赋值函数以优化, 如果注释掉下面的宏, 则使用拷贝赋值函数
// 拷贝复制 = 拷贝构造 + 移动复制
// 但是需要显式调用构造函数或者std::move()
#define DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE

template <size_t Idx, typename... Ts>
struct _tuple;

namespace internal {

template <typename T>
struct get_type {
    using type = T;
};

} // namespace internal

#define __merge__(x, y) x##y
#define __merge_tow_args__(x, y) __merge__(x, y)
#define test_bool_to_void(...)                                          \
    constexpr bool __merge_tow_args__(if_, __LINE__) = (__VA_ARGS__);   \
    HX::print::println(#__VA_ARGS__, " -> ", __merge_tow_args__(if_, __LINE__))

int __test_is_tuple__ = [] {
    return 0;

    test_bool_to_void(std::is_same_v<std::tuple<int>, std::tuple<int&>>);
    test_bool_to_void(std::is_same_v<_tuple<0, int>, _tuple<0, _tuple<0, int>>>);
    
    std::tuple<std::tuple<int>> std_tt{{0}};
    auto std_tt_v = std::tuple<std::tuple<int>>{std_tt};
    (void)std_tt_v;
    return 0;
} ();

template <size_t Idx>
struct _tuple<Idx> {};

template <size_t Idx, typename T>
struct _tuple_head_base {
    T _head;

    constexpr _tuple_head_base() noexcept
        : _head()
    {}

    constexpr _tuple_head_base(const T& t) noexcept
        : _head(t)
    {}

    constexpr _tuple_head_base(const _tuple_head_base&) = default;
    constexpr _tuple_head_base(_tuple_head_base&&) = default;

    template <typename U>
    constexpr _tuple_head_base(U&& t) noexcept
        : _head(std::forward<U>(t))
    {}

    static constexpr const T& _get_head(const _tuple_head_base& t) noexcept {
        return t._head;
    }

    static constexpr T& _get_head(_tuple_head_base& t) noexcept {
        return t._head;
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
        : _tail()
        , _head()
    {}

    constexpr explicit _tuple(const T& head, const Ts&... tail)
        : _tail(tail...)
        , _head(head)
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr explicit _tuple(U&& head, Us&&... tail)
        : _tail(std::forward<Us>(tail)...)
        , _head(std::forward<U>(head))
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr explicit _tuple(const _tuple<Idx, U, Us...>& t) noexcept
        : _tail(_tuple<Idx, U, Us...>::_get_tail(t))
        , _head(_tuple<Idx, U, Us...>::_get_head(t))
    {}

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr explicit _tuple(_tuple<Idx, U, Us...>&& t) noexcept
        : _tail(std::move(_tuple<Idx, U, Us...>::_get_tail(t)))
        , _head(std::move(_tuple<Idx, U, Us...>::_get_head(t)))
    {}

    constexpr _tuple(const _tuple&) = default;
    constexpr _tuple(_tuple&&) = default;

#ifdef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE
    constexpr _tuple& operator=(const _tuple&) = delete; // 删除默认拷贝赋值
#endif
#ifndef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE
    constexpr _tuple& operator=(const _tuple&) = default;
#endif
    constexpr _tuple& operator=(_tuple&&) = default;

    static constexpr const T& _get_head(const _tuple& t) noexcept {
        return _head::_get_head(t);
    }

    static constexpr T& _get_head(_tuple& t) noexcept {
        return _head::_get_head(t);
    }

    // 引用多态
    static constexpr const _tail& _get_tail(const _tuple& t) noexcept {
        return t;
    }

    static constexpr _tail& _get_tail(_tuple& t) noexcept {
        return t;
    }

    // assign 赋值
#ifndef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE
    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr void _assign(const _tuple<Idx, U, Us...>& that) noexcept {
        _get_head(*this) = _tuple<Idx, U, Us...>::_get_head(that);
        if constexpr (sizeof...(Us) > 0) {
            _tuple<Idx + 1, Ts...>::_assign(_tuple<Idx, U, Us...>::_get_tail(that));
        }
    }
#endif

    template <typename U, typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr void _assign(_tuple<Idx, U, Us...>&& that) noexcept {
        _get_head(*this) = std::move(_tuple<Idx, U, Us...>::_get_head(that));
        if constexpr (sizeof...(Us) > 0) {
            _tail::_assign(std::move(_tuple<Idx, U, Us...>::_get_tail(that)));
        }
    }
};

template <typename... Ts>
struct tuple : public _tuple<0, Ts...> {
    using _base = _tuple<0, Ts...>;

    template <typename... Us>
    static constexpr bool _constructible() noexcept {
        if constexpr (sizeof...(Ts) != sizeof...(Us)) {
            return false;
        } else {
            // 注意区分: is_constructible 与 is_convertible
            // 1) is_constructible<T, U> 是否能用 U 类型的对象显式地构造 T, 即 T t = T(u); 是否有效
            // 2) is_convertible<T, U>  是否可以将 U 隐式转换为 T, 即 T t = u; 是否有效
            return std::conjunction_v<std::is_constructible<Ts, Us>...>;
        }
    }

    template <typename... Us>
    static constexpr bool _disambiguating_constraint() noexcept {
        if constexpr (sizeof...(Ts) != sizeof...(Us)) {
            return false;
        } else if constexpr (sizeof...(Us) == 1) {
            using _U0 = typename std::_Nth_type<0, Us...>::type;
	        return !std::is_same_v<std::remove_cvref_t<_U0>, tuple>;
        }
        return true;
    }

    constexpr tuple()
        : _base()
    {}

    template <typename = std::enable_if_t<
        _constructible<const Ts&...>()>>
    constexpr tuple(const Ts&... ts)
        : _base(ts...)
    {}

    template <typename... Us, 
        typename = std::enable_if_t<(
            _constructible<Us...>()
            // && _disambiguating_constraint<Us...>() // 好像不起作用... (略显多余)
        )>>
    constexpr tuple(Us&&... ts)
        : _base(std::forward<Us>(ts)...)
    {}

    // 为了满足类型转换(可强转) int -> std::size_t
    template <typename... Us, 
        typename = std::enable_if_t<_constructible<Us...>()>>
    constexpr tuple(const tuple<Us...>& t)
        : _base(static_cast<const _tuple<0, Us...>&>(t)) // 此处需要转换类型为基类
    {}                                                   // 否则模版不匹配, 就会匹配到 Us&& 上

    template <typename... Us, 
        typename = std::enable_if_t<_constructible<Us...>()>>
    constexpr tuple(tuple<Us...>&& t)
        : _base(std::move(static_cast<_tuple<0, Us...>&&>(t)))
    {}

    constexpr tuple(const tuple&) = default;
    constexpr tuple(tuple&&) = default;

#ifdef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE
    constexpr tuple& operator=(const tuple&) = delete; // 禁止拷贝复制
#endif
#ifndef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE
    constexpr tuple& operator=(const tuple&) = default; // 禁止拷贝复制
#endif
    constexpr tuple& operator=(tuple&&) = default;

#ifndef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE
    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us) 
            && !std::is_same_v<tuple<Ts...>, tuple<Us...>>)>>
    constexpr tuple& operator=(const tuple<Us...>& that) noexcept {
        this->template _assign<Us...>(that);
        return *this;
    }
#endif

    template <typename... Us, 
        typename = std::enable_if_t<(sizeof...(Ts) == sizeof...(Us))>>
    constexpr tuple& operator=(tuple<Us...>&& that) noexcept {
        _base::_assign(std::move(that));
        return *this;
    }
};

template <typename... Ts>
tuple(Ts...) -> tuple<Ts...>;

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

namespace internal {

// 这个`reference_wrapper`配合`ref`的, 以后再搞, 涉及到一个类型等价判断的...
template <typename T>
struct reference_wrapper {
    constexpr reference_wrapper(T& _t) : t(_t) {}

    T& t;
};

// 推导规则
template <typename T>
reference_wrapper(T&) -> reference_wrapper<T>;

template <typename T>
struct reference_wrapper_and_strip {
    using type = T;
};

template <typename T>
struct reference_wrapper_and_strip<std::reference_wrapper<T>> { // 如果有 std::reference_wrapper 才是引用
    using type = T&;                                            // std::reference_wrapper 是 std::ref 的返回值
};

template <typename T>
using decay_and_strip_v = reference_wrapper_and_strip<std::decay_t<T>>::type;

} // namespace internal

template <typename T>
inline constexpr internal::reference_wrapper<T> ref(T& t) noexcept {
    return internal::reference_wrapper<T>{t};
}

// 标准库的 make_tuple 默认推导 Ts... 是不带 const 和 v 以及 (&)[] 的
template <typename... Ts>
constexpr tuple<typename internal::decay_and_strip_v<Ts>...> make_tuple(Ts&&... ts) noexcept {
    using resType = tuple<typename internal::decay_and_strip_v<Ts>...>;
    return resType{std::forward<Ts>(ts)...};
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
    template <typename U = T, 
        typename = std::enable_if_t<std::conjunction_v<std::is_convertible<U, T>>>>
    constexpr Test(U&& _t)
        : t(std::forward<U>(_t)) // 这里 forward 用 U 以保持值类别
    {}

    template <typename U>
    constexpr Test<T>& operator=(const Test<U>& that) noexcept {
        t = that.t;
        return *this;
    }

    constexpr Test(const Test&) = default;
    constexpr Test(Test&&) = default;

    constexpr Test& operator=(const Test&) = delete;
    constexpr Test& operator=(Test&&) = default;

    T t;
};

int __test__ = []() {{
        int a = 1;
        Test<int> t1 (1);
        Test<int&> t2 (a);
        t1 = t2;
    } {
        int a = 1;
        Test<int&> t1 (a);
        Test<int> t2 (1);
        t1 = t2;
    } {
        Test<int> t1(1);
        Test<int> t2(2);
        t1 = Test<int>{t2};
    }
    return 0;
} ();

int main() {
    // 特别注意
    test_bool_to_void(tuple<int&>::_constructible<int>()); // false
                                                           // int& 不能从int构造 (因为int&是左值, int是右值)

    test_bool_to_void(tuple<int>::_constructible<int&>()); // true
                                                           // int 可以从int&构造 (复制值)

    tuple<int, double, tuple<std::string, int>> t{
        1, 
        3.14,
        {"awa", 1}
    };

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
        int& i = a, j = b;
        int k = c;
        auto mp = ::make_tuple(i, std::ref(j), k);
        auto std_mp = std::make_tuple(i, std::ref(j), k);
        i = 114;
        j = 514;
        k = 666;
        HX::print::println("<HX>  auto [i, j, k] = ", get<0>(mp), " ", get<1>(mp), " ", get<2>(mp));
        HX::print::println("<std> auto [i, j, k] = ", get<0>(std_mp), " ", get<1>(std_mp), " ", get<2>(std_mp));
    }

    // 各种拷贝、移动、赋值的测试
    {
        int a = 123;
        tuple<int> t1{int{}};
        tuple<int> t2{3};
        {
            tuple<int&> t3{a};
            tuple<int> t4{t3};
        } {
            tuple<int> t3{a};
            // tuple<int&> t4{t3};
        }

        {
            std::tuple<int&> t3{a}; 
            std::tuple<int> t4{t3}; 
        } {
            std::tuple<int> t3{a}; 
            // std::tuple<int&> t4{t3};
        }

#ifdef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE 
        t1 = std::move(t2);
        t2 = tuple<int>{t1};
#endif
#ifndef DELETE_COPY_ASSIGNMENT_FUNCTION_TO_OPTIMIZE 
        t1 = t2;
        t2 = t1;
#endif

        {
#if 0
            HX::print::println("std::conjunction_v<std::is_convertible<int, int&>> ", 
                std::conjunction_v<std::is_convertible<int, int&>>);
            HX::print::println("std::conjunction_v<std::is_convertible<tuple<int>, tuple<int&>>> ", 
                std::conjunction_v<std::is_convertible<tuple<int>, tuple<int&>>>);
            HX::print::println("std::conjunction_v<std::is_convertible<std::tuple<int>, std::tuple<int&>>> ", 
                std::conjunction_v<std::is_convertible<std::tuple<int>, std::tuple<int&>>>);
#endif
        }
        // (void)t4;
    }

    // 嵌套tuple测试
    {
        auto t = tuple<int>{1};
        tuple<tuple<int>> tt{std::move(t)};
        auto tt_2 = tuple<tuple<int>>{tt};

        HX::print::println("<0, 0>: ", get<0>(get<0>(tt_2)));
    }

    {
#if 0 // 如果类型不匹配, 会报错没有这个构造函数, 而不是没有在内部赋值时候报错 (也就是被匹配上了)
        auto t = std::tuple<int>{1};
        std::tuple<std::tuple<int>> tt{std::move(t)};
        auto tt_2 = std::tuple<std::tuple<int, int>>{tt};

        HX::print::println("<0, 0>: ", get<0>(get<0>(tt_2)));
        HX::print::println("<0, 1>: ", get<1>(get<0>(tt_2)));
#endif
    }
    return 0;
}