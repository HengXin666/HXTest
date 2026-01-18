#include <HXTest.hpp>

struct A {
    A& operator=(A&&) noexcept {
        log::hxLog.info("移动");
        return *this;
    }

    A& operator=(A const&) {
        log::hxLog.info("拷贝");
        return *this;
    }
};

template <typename... Ts>
struct Test {
    using Type = std::tuple<Ts&&...>;
    Type _tp;

    Test(Ts&&... ts)
        : _tp{std::forward<Ts>(ts)...}
    {}
};

template <typename... Ts>
struct Test2 {
    using Type = std::tuple<Ts...>;
    Type _tp;

    Test2(Ts... ts)
        : _tp{std::forward<Ts>(ts)...}
    {}
};

template <typename... Ts>
constexpr auto func(Ts&&... ts) {
    return Test<Ts...>{std::forward<Ts>(ts)...};
}

template <typename... Ts>
constexpr auto func2(Ts&&... ts) {
    return Test2<Ts&&...>{std::forward<Ts>(ts)...};
}

template <typename T>
constexpr T sb(T&& t) {
    return std::forward<T>(t);
}

int main() {
    std::string str{"data"};
    A a{};
    meta::remove_cvref_t<decltype(func(a, A{}))>::Type 
        _{a, A{}};

    meta::remove_cvref_t<decltype(func2(a, A{}))>::Type 
        _2{a, A{}};

    using sb_1 = decltype(sb(a));
    using sb_2 = decltype(sb(A{}));
    using sb_3 = decltype(sb<A&&>(A{}));
}

/*
总结: 完美转发的推导只能是非指定模板参数情况, 
    如 foo<T>() 就不是完美转发了
    只能是 foo(t) 进行参数推导

    template <typename T> T func();
    template <typename T> void foo(T&&);

    其中, foo(1) 的 T = [int&] (推导为 int& &&, 折叠后为 int&)
    但是, foo(int{}) 的 T = [int] (推导为 int&&, T&& 取 T 为 int)

如果期望继续完美转发, 建议优先 forward, 基于参数推导.

如果是类模板, 不方便参数推导情况. 需要指定参数. 

1) 指定为 Class<T&&> ( Class<decltype(std::forward<T>(t))> 同理 )
    这样 T = [int&], 指定为 Class<int& &&> -> Class<int&>
    同理 T = [int ], 指定为 Class<int  &&> -> Class<int&&>

    注意, foo(int{}) 这种传递方法, 内部如果使用的话, 生命周期仅在 `foo(int{})` 这一行
    如果作为返回值保存了 int{} 的 int&&, 那么会悬挂引用 (除非内部构造新的)

2) 指定为 Class<T>
    这样 T = [int&], 指定为 Class<int&> -> Class<int&>
    同理 T = [int ], 指定为 Class<int > -> Class<int>

    安全的返回, 坏处是 原本 int&& 的情况变为 int 了, 如果是类类型, 会导致拷贝开销

结论:
    推荐使用 Class<T&&> 指定. 
    - 如果仅原地视图, 不导出. 那么随便使用.
    - 如果作为返回值等存储. 那么需要原地构造用法
*/