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
 * @return  
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
        return member_count<T, Args..., Any>();
    }
}

int main() {
    std::cout << member_count<Person>() << '\n';

    // Person p3 {Any{}, Any{}, Any{}}; // 报错

    // 可以使用静态模版for 展开, 以得到正确的 这个数量
    // 然后使用结构化绑定 auto&& [...] 来获取到对应的东西
    // 然后 模板函数的__PRETTY_FUNCTION拿到成员名字
    // 即可对于到字符串上, 同理反之
    // 以实现无宏反射!!!

    return 0;
}