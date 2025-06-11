#include <HXprint/print.h>

// noexcept 表示不抛出异常
void fun1() noexcept {
    // throw ""; noexcept函数中, 如果抛出异常, 那么程序会直接终止!
}

// noexcept(表达式), 表达式为true, 则表示不抛出异常. 注意: 表达式需要为常量表达式
void fun2() noexcept(1) {
    
}

// tip: 默认函数不写 noexcept, 等价于 写了 noexcept(false)
void fun3() noexcept(false) {
    
}

// tip: throw() 等价于 noexcept, 但是是C++98的写法, 是老东西!
void fun4() throw() {

}

struct Awa {
    Awa(int) {} // 默认为 noexcept(false)
    void fun() {}
    void funNE() noexcept {}
    ~Awa() {} // 默认为 noexcept
};

struct B {
    B(int) noexcept(true) {} // 如果析构为noexcept(false), 则构造也为noexcept(false)
                             // 更具体的, 请见下面
    ~B() noexcept(false) {}
};

struct C {
    C(int) : _(0) {}
    ~C() {}

    // 拷贝构造涉及到成员的构造, 因此会推导为 noexcept(false)
    // 拷贝赋值不涉及析构, 因此会推导为 noexcept(true)
private:
    B _; // 成员的析构为 noexcept(false), 因此本类的构造和析构都为 noexcept(false)
};
// 具体细节:
// https://cppreference.cn/w/cpp/language/noexcept_spec


// 支持编译期模版
template <typename T>
void funTemplate() noexcept(std::same_as<T, int>) {}

int main() {
    try {
        fun1();
    } catch (...) {
        HX::print::println("有异常!");
    }
    HX::print::println("A (默认的情况):");
    HX::print::println("noexcept(A{int}) = ", noexcept(Awa{0}));
    HX::print::println("noexcept(std::declval<A>().fun()) = ", noexcept(std::declval<Awa>().fun()));
    HX::print::println("noexcept(std::declval<A>().funNE()) = ", noexcept(std::declval<Awa>().funNE()));
    HX::print::println("noexcept(std::declval<A>().~A()) = ", noexcept(std::declval<Awa>().~Awa()));
    
    HX::print::println("");
    HX::print::println("B (自定义的情况):");
    HX::print::println("noexcept(B{int}) = ", noexcept(B{0}));
    HX::print::println("noexcept(std::declval<B>().~B()) = ", noexcept(std::declval<B>().~B()));

    HX::print::println("");
    HX::print::println("C (成员的析构存在非noexcept的情况):");
    HX::print::println("noexcept(C{int}) = ", noexcept(C{0}));
    HX::print::println("noexcept(std::declval<C>().~C()) = ", noexcept(std::declval<C>().~C()));
    HX::print::println("noexcept(C的拷贝构造) = ", noexcept(C(std::declval<C>())));
    // HX::print::println("noexcept(C的移动构造) = ", noexcept(C(std::move(std::declval<C>()))));
    HX::print::println("noexcept(C的拷贝赋值) = ", noexcept(std::declval<C>().operator=(std::declval<C>())));
    // HX::print::println("noexcept(C的移动赋值) = ", noexcept(std::declval<C>().operator=(std::move(std::declval<C>()))));

    // 模版编译期表达式求 noexcept
    HX::print::println("");
    HX::print::println("编译期表达式求 noexcept:");
    HX::print::println("funTemplate<int>: ", noexcept(funTemplate<int>()));
    HX::print::println("funTemplate<A>: ", noexcept(funTemplate<Awa>()));

    // lambda 也可以声明为 noexcept
    auto lambda_noexcept_true = []() noexcept(true) {};
    auto lambda_noexcept_false = []() noexcept(false) {};
    HX::print::println("");
    HX::print::println("lambda 也可以声明为 noexcept:");
    HX::print::println("noexcept(true) lambda: ", noexcept(lambda_noexcept_true()));
    HX::print::println("noexcept(false) lambda: ", noexcept(lambda_noexcept_false()));
    return 0;
}
