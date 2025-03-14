#include <HXprint/print.h>

#include <tuple>
#include <vector>

/**
 * @brief 对标的是 std::integral_constant<std::size_t, N>
 * @tparam N 
 */
template <std::size_t N>
struct int_constexpr {
    // 编译期常量, 用于作为模版参数
    inline constexpr static std::size_t value = N;

    // 编译期隐式类型转换
    constexpr operator std::size_t() const noexcept {
        return value;
    }
};

/**
 * @brief 对标的是 std::index_sequence<size_t... Idx>
 * @tparam N 
 */
template <std::size_t... N>
struct index_constexpr {
    // 编译期隐式类型转换
    constexpr operator std::size_t() const noexcept {
        return sizeof...(N);
    }
};

template <std::size_t Num>
struct _make_index_constexpr {
    template <std::size_t N, std::size_t... Seq>
    static constexpr auto recursion() noexcept {
        if constexpr (N == Num) {
            return index_constexpr<Seq...>{};
        } else {
            return recursion<N + 1, Seq..., N>();
        }
    }
};

/**
 * @brief 对标 std::make_index_sequence<size_t _Num>
 * @brief 注意, 标准库中使用了编译器的特殊优化, 如果没有使用才可能是递归实现 (反正我看不到它的实现)
 * @tparam Num 
 */
template <std::size_t Num>
using make_index_constexpr = decltype(_make_index_constexpr<Num>::template recursion<0>());

/**
 * @brief 编译期for循环 (递归实现), 范围是: [0, N)
 * @tparam N 
 * @param fun 传入的是 索引
 */
template <std::size_t N>
inline constexpr void static_for(auto&& fun) {
    if constexpr (N > 0) {
        static_for<N - 1>(fun);
        fun(int_constexpr<N - 1>{});
    }
}

/**
 * @brief 编译期for循环 (迭代实现 [准确来说是`逗号表达式展开`]), 范围是: [0, N)
 * @tparam Is 
 * @param fun 传入的是 索引
 */
template <std::size_t... Is>
inline constexpr void staticFor(std::index_sequence<Is...>, auto&& fun) {
    (fun(int_constexpr<Is>{}), ...);
}

// === variant 用到的 === {
// 需要使用一个std::vector把std::variant的东西装起来
// 而直接 std::vector<std::variant<Ts...>> 会非常占用空间
// 并且不能被矢量化 (遍历的时候)
// 因此我们可以把他们按照类型分开为 std::tuple<std::vector<Ts>...>

// 主模版
template <typename V>
struct variant_to_vecotr_by_tuple;

// 偏特化
template <typename... Ts>
struct variant_to_vecotr_by_tuple<std::variant<Ts...>> {
    using type = std::tuple<std::vector<Ts>...>;
};

template <typename V>
using ObjList = variant_to_vecotr_by_tuple<V>::type;

// } === variant 用到的 ===

// === variant 的 std::visit 的魔法 === {

template <typename... Ts>
constexpr auto visit(auto&& fun, std::variant<Ts...>&& vt) {
    // ps: 还需要检查运行时 variant 的状态是否合法, 另外标准库实际上是支持多个 vt 调用的哦!
    // 此处只是简单的说明了一下, visit 的本质实际上就是 for展开然后匹配运行时索引
    if constexpr (std::is_void_v<decltype(fun(std::get<0>(vt)))>) {
        auto _fun = [&](auto idx) {
            fun(std::get<idx>(vt));
            return true;
        };
        [&] <size_t... Is> (index_constexpr<Is...>) {
            ((Is == vt.index() && _fun(int_constexpr<Is>{})) || ...); // 短路求值优化
        }(make_index_constexpr<sizeof...(Ts)>{});
        return;
    } else {
        decltype(fun(std::get<0>(vt))) res;
        auto _fun = [&](auto idx) {
            res = fun(std::get<idx>(vt));
            return true;
        };
        [&] <size_t... Is> (index_constexpr<Is...>) {
            ((Is == vt.index() && _fun(int_constexpr<Is>{})) || ...);
        }(make_index_constexpr<sizeof...(Ts)>{});
        return res;
    }
}

// } === variant 的 std::visit 的魔法 ===

int main() {
    // 朴素的
    {
        HX::print::println("朴素的遍历并且输出索引:");
        static_for<3>([](auto&& idx) {
            HX::print::println(idx.value);
        });
    }

    // 遍历tuple
    {
        std::tuple<int, double, std::string> tp{1, 3.14, "你好"};
        auto for_tuple = [](auto&& tp) {
            // 递归的静态编译期for
            HX::print::println("\n遍历tuple: 递归的静态编译期for:");
            static_for<std::tuple_size_v<std::remove_reference_t<decltype(tp)>>>([&](auto idx) {
                HX::print::println(idx.value, " -> ", std::get<idx>(tp));
            });
            
            // 迭代的静态编译期for
            HX::print::println("\n遍历tuple: 迭代的静态编译期for");
            staticFor(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<decltype(tp)>>>{},
                [&](auto idx) {
                HX::print::println(idx.value, " -> ", std::get<idx>(tp));
            });

            // C++20 支持匿名模版, 也可实现迭代的静态编译期for
            HX::print::println("\n遍历tuple: C++20 支持匿名模版, 也可实现迭代的静态编译期for");
            [&] <std::size_t... Is>(std::index_sequence<Is...>, auto&& fun) {
                (fun(int_constexpr<Is>{}), ...);
            }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<decltype(tp)>>>{},
                [&](auto idx) {
                HX::print::println(idx.value, " -> ", std::get<idx>(tp));
            });

            // 同理, 您也可以自定义以支持逗号表达式展开
            HX::print::println("\n遍历tuple: C++20 支持匿名模版, 也可实现迭代的静态编译期for (自定义支持逗号表达式展开)");
            [&] <std::size_t... Is>(index_constexpr<Is...>, auto&& fun) {
                (fun(int_constexpr<Is>{}), ...);
            }(make_index_constexpr<std::tuple_size_v<std::remove_reference_t<decltype(tp)>>>{},
                [&](auto idx) {
                HX::print::println(idx.value, " -> ", std::get<idx>(tp));
            }); 
        };
        for_tuple(tp);
    }

    // (某些情况下的静态多态) 比如你是一个闭源公司的痞劳保
    // 不希望把你的代码暴露出来 (毕竟是使用了左移运算符这种高精尖的复杂位运算代码, 妥妥的商业机密!)
    // 即便是 template <typename T>void show(T&& t) {std::cout << t << std::endl;}
    // 但是, 模版这种东西, 得写到头文件, 那不是泄漏商业机密了吗?! 很快就会破产的! 許さない!
    // 可以[前向声明模版在头文件], 然后在cpp中实现, 再特化调用一下, 也不是不行, 但是它就不是万能模版了,
    // 有支持范围的了
    //
    // 但是, 实际上有一种更加好的办法, 就是在头文件中声明下面这个`Obj`类型
    // 然后cpp中只需要这样`add_obj`、`print_obj`的实现, 即可
    // 如果需要存储数据, 可以参考 ObjList<Obj>
    // 这样可以在尽可能的编译期的情况下, 不暴露代码. 同时比头文件的纯模版写法, 减小了二进制膨胀的问题
    // 唯一不好的就是, 它依旧只支持它支持的类型 (即Obj中的类型)
    {
        HX::print::println("\n静态多态:");
        using Obj = std::variant<int, double, std::string>;
        ObjList<Obj> list;

        // 标准库的 visit
        HX::print::println("\n标准库的 visit:");
        std::visit([](auto&& v) {
            HX::print::println("visit: ", v);
        }, Obj{"你好"});

        
        // 自己实现的简易 visit
        HX::print::println("\n自己实现的简易 visit:");
        auto res = visit([](auto&& v) {
            HX::print::println("visit: ", v);
            return std::vector{1, 2, 3};
        }, Obj{"你好"});
        HX::print::println("\n可以返回你喜欢的东西: ", res);

        auto add_obj = [&](Obj&& obj) {
            static_for<std::variant_size_v<Obj>>([&](auto idx) {
                if (obj.index() == idx) {
                    std::get<idx>(list).push_back(std::get<idx>(obj));
                }
            });
        };

        auto print_obj = [&]() {
            static_for<std::variant_size_v<Obj>>([&](auto idx) {
                for (auto const& v : std::get<idx>(list)) {
                    HX::print::print(v, ' ');
                }
                HX::print::print("\n");
            });
        };

        add_obj(Obj{1});
        add_obj(Obj{2});
        add_obj(Obj{3.14});
        add_obj(Obj{12.56});
        add_obj(Obj{"Heng"});
        add_obj(Obj{"Xin"});

        HX::print::println("\n遍历结果 print_obj:");
        print_obj();

        // (虽然咱们HX::print也可以输出就是了, 但是实际上使用上面的场景, 一般是进行数据运算的qwq)
        HX::print::println(list);
    }
    return 0;
}