#include <iostream>
#include <string>
#include <string_view>

struct Man {
    int id;
    std::string name;
};

struct Any {
    template <class T>
    operator T();
};


template <class T, class... Args>
consteval auto cnt() {
    if constexpr (requires {
        T {Args{}..., Any{}};
    }) {
        return cnt<T, Args..., Any>();
    } else {
        return sizeof...(Args);
    }
}

constexpr void visitMembrs(auto&& obj, auto&& visit) {
    using T = std::remove_cv_t<std::remove_reference_t<decltype(obj)>>;
    constexpr auto Cnt = cnt<T>();
    if constexpr (Cnt == 0) {
        return visit();
    } else if constexpr (Cnt == 1) {
        auto&& [a1] = obj;
        return visit(a1);
    } else if constexpr (Cnt == 2) {
        auto&& [a1, a2] = obj;
        return visit(a1, a2);
    }
    // ...
}

template <auto Val>
void loli() {
    std::cout << "\n\n\n" << __PRETTY_FUNCTION__ << '\n';
}

int main() {
    // C++11 聚合初始化
    if (0) {
        std::cout << cnt<Man>() << '\n';
    }
    // C++17 结构化绑定
    if (0) {
        auto fun = [](int a, std::string& b) {
            std::cout << a << ' ' << b << '\n';
        };
        
        Man man{1, "2233"};
        visitMembrs(man, fun);
    }

    // 编译器方言 + auto占位非类型模版形参 + 符号指针 (编译期指针/常量表达式指针)
    {
        static const Man man {0, "0"};
        loli<&man.id>();
    }
    return 0;
}