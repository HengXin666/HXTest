#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

struct Person {
    int id;
    std::string name;

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
template <class T>
inline constexpr auto wrap(T const &arg) noexcept {
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
    static inline remove_cvref_t<T> value;
};

template <class T>
inline constexpr remove_cvref_t<T> &get_fake_object() noexcept {
    return wrapper<remove_cvref_t<T>>::value;
}

// 此处的 T = Person
#define RFL_INTERNAL_OBJECT_IF_YOU_SEE_AN_ERROR_REFER_TO_DOCUMENTATION_ON_C_ARRAYS(n, ...) \
    template <class T>                                                                     \
    struct object_tuple_view_helper<T, n> {                                                \
        static constexpr auto tuple_view() {                                               \
            (void)"// 看来 核心就是下面这个 get_fake_object 的作用! (现在我就是不明白, get_fake_object干了什么qwq...)"; \
            (void)"// 先是将他们打包, 然后结构化绑定 到__VA_ARGS__"; \
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

template <typename T>
inline constexpr std::array<std::string_view, members_count_v<T>>
get_member_names() {
    auto arr = _get_member_names<T>();
    using U = remove_cvref_t<T>;
    if constexpr (has_alias_field_names_v<U> ||
                  has_inner_alias_field_names_v<U>) {
        constexpr auto alias_arr = get_alias_field_names<U>();
        for (size_t i = 0; i < alias_arr.size(); i++) {
            arr[alias_arr[i].index] = alias_arr[i].alias_name;
        }
    }
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

int main() {
    auto vistPrint = [](auto &&...its) {
        ((std::cout << its << ' '), ...);
        std::cout << '\n';
    };

    Person p{2233, "114514"};
    visit_members(p, vistPrint);

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
