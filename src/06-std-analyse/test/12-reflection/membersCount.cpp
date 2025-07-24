#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <any>
#include <tuple>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <stack>
#include <queue>
#include <bitset>
#include <memory>
#include <chrono>
#include <complex>
#include <utility>
#include <type_traits>

template <typename T>
constexpr bool isNonClassType = !std::is_class_v<T> && !std::is_union_v<T>;

template <typename T, typename U, typename... Args>
constexpr bool isConstructible = requires {
    T {{Args{}}..., {U{}}};
};

struct Any {
    template <typename T>
    operator T();
};

struct AnyOpt {
    template <typename T>
        requires(requires(T t) {
            typename T::value_type;
            t.has_value();
            t.value();
            t.operator*();
        })
    operator T();
};

struct AnyPtr {
    // template <typename T>
    // requires(requires(T t) {
    //         typename T::element_type;
    //         t.reset();
    //         t.get();
    //         t.operator*();
    //         t.operator->();
    //         { static_cast<bool>(t) } -> std::same_as<bool>;
    //     })
    // operator T();

    operator std::nullptr_t(); // 智能指针 / GCC std::string_view 都匹配这个
};

template <typename T, typename... Args>
inline constexpr std::size_t membersCountImpl() {
    if constexpr (isConstructible<T, Any, Args...>) {
        return membersCountImpl<T, Args..., Any>();
    } else if constexpr (isConstructible<T, AnyOpt, Args...>) {
        return membersCountImpl<T, Args..., AnyOpt>();
    } else if constexpr (isConstructible<T, AnyPtr, Args...>) {
        return membersCountImpl<T, Args..., AnyPtr>();
    } else {
        return sizeof...(Args);
    }
}

template <typename T>
inline constexpr std::size_t membersCount() {
    if constexpr (std::is_aggregate_v<T>) {
        return membersCountImpl<T>();
    } else {
        static_assert(!sizeof(T), "不是聚合类");
    }
}

int main() {
    struct AJson {
        std::initializer_list<int> l;
        std::shared_ptr<std::string> e;
        std::optional<std::string> d;
        std::shared_ptr<std::string> e2;
        std::optional<std::string> d2;
        std::shared_ptr<std::string> e3;
        std::optional<std::string> d3;
    };
    constexpr auto N = membersCountImpl<AJson>();
    printf("N = %zu\n", N);
    return 0;
}
