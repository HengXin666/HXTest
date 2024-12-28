#include <iostream>
#include <string>

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
     * 如果模版 T {Args{}..., Any{}} 不能被实例化, 说明 T 的成员参数个数 == sizeof...(Args)
     */
    if constexpr (requires { T{ {Args{}}..., {Any{}} }; } == false) {
        return sizeof...(Args);
    } else {
        // 如果可以实例化, 则添加一个参数
        return member_count<T, Args..., Any>();
    }
}

constexpr decltype(auto) visit_members(auto&& obj, auto&& visitor) {
    // 去除引用, 获取实际类型
    using ObjType = std::remove_reference_t<decltype(obj)>;
    constexpr auto Cnt = member_count<ObjType>();

    if constexpr (Cnt == 0) {
        return visitor();
    } else if constexpr (Cnt == 1) {
        auto&& [a1] = obj;
        return visitor(a1);
    } else if constexpr (Cnt == 2) {
        auto&& [a1, a2] = obj;
        return visitor(a1, a2);
    } else if constexpr (Cnt == 3) {
        auto&& [a1, a2, a3] = obj;
        return visitor(a1, a2, a3);
    } // ...
    throw;
}

int main() {
    auto vistPrint = [](auto&&... its) {
        ((std::cout << its << ' '), ...);
        std::cout << '\n';
    };

    Person p{2233, "114514"};
    visit_members(p, vistPrint);

    // Person p3 {Any{}, Any{}, Any{}}; // 报错

    // 可以使用静态模版for 展开, 以得到正确的 这个数量
    // 然后使用结构化绑定 auto&& [...] 来获取到对应的东西
    // 然后 模板函数的__PRETTY_FUNCTION拿到成员名字
    // 即可对于到字符串上, 同理反之
    // 以实现无宏反射!!!

    return 0;
}