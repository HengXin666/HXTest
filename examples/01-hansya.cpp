#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

struct Person {
    int id____;
    std::string name____;

    // 需要使用聚合初始化, 因此不能书写构造函数
    // Person() {}
};

struct Any {
    template <class T>
    operator T(); // 重载了类型转化运算符
};

/*
首先利用聚合初始化 sfiane 出成员个数，
然后结构化绑定拿到成员，
通过模板函数的__PRETTY_FUNCTION拿到成员名字。
*/

/**
 * @brief 编译期静态递归计算结构体成员个数
 *
 * @tparam T
 * @tparam Args
 * @return 结构体成员个数
 */
template <class T, class... Args>
consteval auto member_count() {
    // consteval 是C++20 的强制编译期执行操作, 如果编译期没有执行则会报错
    /**
     * 如果模版 T {Args{}..., Any{}} 不能被实例化, 说明 T 的成员参数个数 ==
     * sizeof...(Args)
     */
    if constexpr (requires { T{{Args{}}..., {Any{}}}; } == false) {
        return sizeof...(Args);
    } else {
        // 如果可以实例化, 则添加一个参数
        return member_count<T, Args..., Any>();
    }
}

/*
对于 Person, 是实例化为: (GCC)
目前问题是, 为什么可以出现 & wrapper<Person>::value.Person::id 这种展示到成员名称的, 成员指针?!

constexpr std::string_view get_member_name() [
    with auto ptr = Wrapper<int*>{
        (& wrapper<Person>::value.Person::id)}; 
        std::string_view = std::basic_string_view<char>],

constexpr std::string_view get_member_name() [
    with auto ptr = Wrapper<std::__cxx11::basic_string<char>*>{
        (& wrapper<Person>::value.Person::name)}; 
        std::string_view = std::basic_string_view<char>], 
*/

template <auto ptr>
inline constexpr std::string_view get_member_name() {
#if defined(_MSC_VER)
    constexpr std::string_view func_name = __FUNCSIG__;
#else
    constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif

#if defined(__clang__)
    auto split = func_name.substr(0, func_name.size() - 2);
    return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
    auto split = func_name.substr(0, func_name.rfind(")}"));
    return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
    auto split = func_name.substr(0, func_name.rfind("}>"));
    return split.substr(split.rfind("->") + 2);
#else
    static_assert(
        false, "You are using an unsupported compiler. Please use GCC, Clang "
               "or MSVC or switch to the rfl::Field-syntax.");
#endif
}

template <typename T>
constexpr std::size_t members_count_v = member_count<T>();

template <class T>
struct Wrapper {
    using Type = T;
    T v;
};

template <class T>
Wrapper(T) -> Wrapper<T>;

// This workaround is necessary for clang.
// 此解决方法对于clang是必要的
template <class T>
inline constexpr auto wrap(const T& arg) noexcept {
    return Wrapper{arg};
}

template <class T, std::size_t n>
struct object_tuple_view_helper {
    static constexpr auto tuple_view() {
        static_assert(
            sizeof(T) < 0,
            "\n\nThis error occurs for one of two reasons:\n\n"
            "1) You have created a struct with more than 100 fields, which is "
            "unsupported. \n\n"
            "2) Your struct is not an aggregate type.\n\n");
    }

    static constexpr auto tuple_view(T &) {
        static_assert(
            sizeof(T) < 0,
            "\n\nThis error occurs for one of two reasons:\n\n"
            "1) You have created a struct with more than 100 fields, which is "
            "unsupported. \n\n"
            "2) Your struct is not an aggregate type.\n\n");
    }

    template <typename Visitor>
    static constexpr decltype(auto) tuple_view(T &&, Visitor &&) {
        static_assert(
            sizeof(T) < 0,
            "\n\nThis error occurs for one of two reasons:\n\n"
            "1) You have created a struct with more than 100 fields, which is "
            "unsupported. \n\n"
            "2) Your struct is not an aggregate type.\n\n");
    }
};

template <class T>
struct object_tuple_view_helper<T, 0> {
    static constexpr auto tuple_view() {
        return std::tie();
    }

    static constexpr auto tuple_view(T &) {
        return std::tie();
    }

    template <typename Visitor>
    static constexpr decltype(auto) tuple_view(T &&, Visitor &&) {}
};

/**
 * @brief 去掉 T 的 &、&&、const、volatile
 * @tparam T
 */
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class T>
struct wrapper {
    static inline remove_cvref_t<T> value {};
};

/**
 * @brief 获取 T 类型的全局静态引用
 * @tparam T 
 * @return constexpr remove_cvref_t<T>& 
 */
template <class T>
inline constexpr remove_cvref_t<T> &get_fake_object() noexcept {
    return wrapper<remove_cvref_t<T>>::value;
}

// 此处的 T = Person
#define RFL_INTERNAL_OBJECT_IF_YOU_SEE_AN_ERROR_REFER_TO_DOCUMENTATION_ON_C_ARRAYS(n, ...) \
    template <class T>                                                                     \
    struct object_tuple_view_helper<T, n> {                                                \
        static constexpr auto tuple_view() {                                               \
            (void)"// 获取 T也就是struct的全局静态引用, 然后将其 成员变量 结构化绑定 到__VA_ARGS__"; \
            auto &[__VA_ARGS__] = get_fake_object<remove_cvref_t<T>>();                    \
            (void)"// 然后使用 tie 将他们按照左值引用, 绑定为 tuple"; \
            auto ref_tup = std::tie(__VA_ARGS__);                                          \
            auto get_ptrs = [](auto &..._refs) {                                           \
                return std::make_tuple(&_refs...);                                         \
            };                                                                             \
            (void)"// 然后调用函数, 以展开, 并且取地址 也就是 成员指针?! (返回是tuple<成员指针...>)"; \
            return std::apply(get_ptrs, ref_tup);                                          \
        }                                                                                  \
    }
        /*
        static constexpr auto tuple_view(T &t) {                                           \
            auto &[__VA_ARGS__] = t;                                                       \
            return std::tie(__VA_ARGS__);                                                  \
        }                                                                                  \
        template <typename Visitor>                                                        \
        static constexpr decltype(auto) tuple_view(T &&t, Visitor &&visitor) {             \
            auto &&[__VA_ARGS__] = t;                                                      \
            return visitor(__VA_ARGS__);                                                   \
        }                                                                                  \
    }
        */

// 使用宏, 生成 object_tuple_view_helper 的 不同n的偏特化!!!
#include "member_macro.hpp"

// template <class T>
// inline constexpr auto tuple_view(T &&t) {
//     return object_tuple_view_helper<T, members_count_v<T>>::tuple_view(t);
// }

// template <size_t Count, class T, typename Visitor>
// inline constexpr decltype(auto) tuple_view(T &&t, Visitor &&visitor) {
//     return object_tuple_view_helper<T, Count>::tuple_view(
//         std::forward<T>(t), std::forward<Visitor>(visitor));
// }

template <class T>
inline constexpr auto struct_to_tuple() {
    return object_tuple_view_helper<T, members_count_v<T>>::tuple_view();
}

template <typename T>
inline constexpr std::array<std::string_view, members_count_v<T>>
_get_member_names() {
    constexpr size_t Count = members_count_v<T>;
    std::array<std::string_view, Count> arr;
#if __cplusplus >= 202002L && (!defined(_MSC_VER) || _MSC_VER >= 1930)
    // 得到 tuple<成员指针...>
    constexpr auto tp = struct_to_tuple<T>();

    // 使用魔法, 遍历每一个成员指针 以实例化模版, to 成员名称, 然后保存到 arr里面
    [&]<size_t... Is>(std::index_sequence<Is...>) mutable {
        ((arr[Is] = get_member_name<wrap(std::get<Is>(tp))>()), ...);
    }(std::make_index_sequence<Count>{});
#else
    init_arr_with_tuple<T>(arr, std::make_index_sequence<Count>{});
#endif
    return arr;
}

/*
template <typename T, typename = void>
struct has_alias_field_names_t : std::false_type {};

template <typename T>
constexpr bool has_alias_field_names_v = has_alias_field_names_t<T>::value;

template <typename T, typename = void>
struct has_inner_alias_field_names_t : std::false_type {};

template <typename T>
constexpr bool has_inner_alias_field_names_v =
    has_inner_alias_field_names_t<T>::value;

template <typename T>
inline constexpr auto get_alias_field_names() {
    if constexpr (has_alias_field_names_v<T>) {
        return get_alias_field_names((T *)nullptr);
    } else if constexpr (has_inner_alias_field_names_v<T>) {
        return T::get_alias_field_names((T *)nullptr);
    } else {
        return std::array<std::string_view, 0>{};
    }
}
*/

template <typename T>
inline constexpr std::array<std::string_view, members_count_v<T>> get_member_names() {
    auto arr = _get_member_names<T>();
    // using U = remove_cvref_t<T>;
    // if constexpr (has_alias_field_names_v<U> ||
    //               has_inner_alias_field_names_v<U>) {
    //     constexpr auto alias_arr = get_alias_field_names<U>();
    //     for (size_t i = 0; i < alias_arr.size(); i++) {
    //         arr[alias_arr[i].index] = alias_arr[i].alias_name;
    //     }
    // }
    return arr;
}

constexpr decltype(auto) visit_members(auto &&obj, auto &&visitor) {
    // 去除引用, 获取实际类型
    using ObjType = std::remove_reference_t<decltype(obj)>;
    constexpr auto Cnt = member_count<ObjType>();

    // 用宏实现!, 见上面
    if constexpr (Cnt == 0) {
        return visitor();
    } else if constexpr (Cnt == 1) {
        auto &&[a1] = obj;
        return visitor(a1);
    } else if constexpr (Cnt == 2) {
        auto &&[a1, a2] = obj;
        return visitor(a1, a2);
    } else if constexpr (Cnt == 3) {
        auto &&[a1, a2, a3] = obj;
        return visitor(a1, a2, a3);
    } // ...
    throw;
}

template <class T>
std::string getName() {
    return __PRETTY_FUNCTION__;
}

template <auto ptr>
inline constexpr std::string getPtrName() {
    return __PRETTY_FUNCTION__;
}

// (全局)静态变量的地址 (指针) 是编译期常量
// static int staticPtr1 = 114514;
// static constexpr int staticPtr2;
// static_assert(
//     &staticPtr1 != &staticPtr2,
//     "报错 => 不是编译期常量"
// );

int main() {
    {
        std::cout << "\n================ 成员值 for-each ================\n";
        auto vistPrint = [](auto &&...its) {
            ((std::cout << its << ' '), ...);
            std::cout << '\n';
        };
        Person p{2233, "114514"};
        visit_members(p, vistPrint);
    }

    {
        std::cout << "\n================ 成员指针 ================\n";
        constexpr int Person::* ptr = &Person::id____;
        // 对于 T
        std::cout << getName<decltype(ptr)>() << "\n";

        // 对于 auto
        std::cout << getPtrName<ptr>() << '\n';
    }

    {
        auto&& [a1, a2] = get_fake_object<Person>();
        std::cout << "\n================ 成员 T ================\n";
        std::cout << getName<decltype(a1)>() << '\n';
        std::cout << getName<decltype(a2)>() << '\n';

        // static_assert(a1 == 0, ""); // 编译器报错了, 不是常量吗? 实则不然!

        // 编译报错: 需要 auto&& [a1, a2] 为常量
        // 但是结构化绑定 不支持声明为编译期常量 (即便他们就是编译期常量)
        // std::cout << "\n================ 成员 auto ================\n";
        // decltype(a1) b1;
        // decltype(a2) b2;
        // std::tie(b1, b2) = get_fake_object<Person>(); // tie只能支持 tuple 和 pair的...
        // std::cout << getPtrName<b1>() << '\n';
        // std::cout << getPtrName<b2>() << '\n';

        // 证明 其成员指针是常量
        static_assert(
            &get_fake_object<Person>().id____ == &get_fake_object<Person>().id____,
            "报错 => 不是编译期常量"
        );
        // static_assert( // 报错
        //     get_fake_object<Person>().id____ != get_fake_object<Person>().id____,
        //     "报错 => 不是编译期常量"
        // );

        std::cout << &get_fake_object<Person>().id____ << '\n';

        constexpr auto ref_tup = std::tie(a1, a2);
        constexpr auto get_ptrs = [](auto &..._refs) {
            // 需要取地址! 不然不是常量!
            // 对 全局静态变量取地址, 得到的是常量
            return std::make_tuple(&_refs...);
        };

        // 此处的 ref_tup 是 tuple<成员...>
        // std::cout << getName<decltype(ref_tup)>() << '\n';
        // std::cout << getPtrName<std::get<0>(ref_tup)>() << '\n';

        // static_assert(std::get<0>(ref_tup) == 0, "");

        // 此处的 res 是 tuple<成员指针...>
        constexpr auto res = std::apply(get_ptrs, ref_tup);
        std::cout << "\n================ tuple<> ================\n";
        std::cout << getName<decltype(res)>() << '\n';

        std::cout << "\n================ get ================\n";
        [&]<size_t... Is>(std::index_sequence<Is...>) mutable {
            ((std::cout << getPtrName<std::get<Is>(res)>() << '\n'), ...);
        }(std::make_index_sequence<2>{});

        // 似乎不需要 wrap ? (GCC) => 因为 Clang 需要? => 好像也可以不需要...
        std::cout << "\n================ wrap + get ================\n";
        [&]<size_t... Is>(std::index_sequence<Is...>) mutable {
            ((std::cout << getPtrName<wrap(std::get<Is>(res))>() << '\n'), ...);
        }(std::make_index_sequence<2>{});
    }

    std::cout << "\n================ 反射 ================\n";
    constexpr auto arr = get_member_names<Person>();
    for (auto name: arr) {
        std::cout << name << ", ";
    }
    std::cout << "\n";

    // Person p3 {Any{}, Any{}, Any{}}; // 报错

    // 可以使用静态模版for 展开, 以得到正确的 这个数量
    // 然后使用结构化绑定 auto&& [...] 来获取到对应的东西
    // 然后 模板函数的__PRETTY_FUNCTION拿到成员名字
    // 即可对于到字符串上, 同理反之
    // 以实现无宏反射!!!

    return 0;
}

/*
讨论: 为什么需要套一层 Wrapper ?
回答: 只是一些元模板编程的规范? || 为了更好的 find 变量的位置?

// Clang
================ get ================
寻找办法:
    1. 去掉最后的 ]
    2. 往前找到 '.' (源码说 ':' 或者 '.')
    3. 取他们之间的
std::string getPtrName() [ptr = &value.id____]
std::string getPtrName() [ptr = &value.name____]

================ wrap + get ================
寻找办法:
    1. 去掉最后的 }]
    2. 往前找到 '.' (源码说 ':' 或者 '.')
    3. 取他们之间的
std::string getPtrName() [ptr = Wrapper<int *>{&value.id____}]
std::string getPtrName() [ptr = Wrapper<basic_string<char, char_traits<char>, allocator<char>> *>{&value.name____}]

// GCC
================ get ================
寻找办法:
    1. 从后往前找 ");"
    2. 往前找到 ':'
    3. 取他们之间的
constexpr std::string getPtrName() [with auto ptr = (& wrapper<Person>::value.Person::id____); std::string = std::__cxx11::basic_string<char>]
constexpr std::string getPtrName() [with auto ptr = (& wrapper<Person>::value.Person::name____); std::string = std::__cxx11::basic_string<char>]

================ wrap + get ================
寻找办法:
    1. 从后往前找 ")}"
    2. 往前找到 ':'
    3. 取他们之间的
constexpr std::string getPtrName() [with auto ptr = Wrapper<int*>{(& wrapper<Person>::value.Person::id____)}; std::string = std::__cxx11::basic_string<char>]
constexpr std::string getPtrName() [with auto ptr = Wrapper<std::__cxx11::basic_string<char>*>{(& wrapper<Person>::value.Person::name____)}; std::string = std::__cxx11::basic_string<char>]

// MSVC (注意, 它的展开方式还不一样)
================ get ================
寻找办法:
    1. 从后往前找 ">"
    2. 往前找到 "->"
    3. 取他们之间的
constexpr std::string getPtrName()<&value->id____>(void)
constexpr std::string getPtrName()<&value->name____>(void)

================ wrap + get ================
寻找办法:
    1. 从后往前找 "}>"
    2. 往前找到 "->"
    3. 取他们之间的
constexpr std::string getPtrName()<struct Wrapper<int *>{int*:&value->id____}>(void)
constexpr std::string getPtrName()<struct Wrapper<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > *>{class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >*:&value->name____}>(void)
*/