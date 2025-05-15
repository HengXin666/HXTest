#include <HXprint/print.h>

#define __test_Forward_Declaration__

/**
 * @brief 此处理论上不算循环依赖, 应该是逻辑错误!
 * 就如同你写链表, 难道可以 struct C { C next; } 吗?
 * 一样的道理!
 */

#ifdef __test_Forward_Declaration__
#if 1
template <typename T>
struct B;

template <typename T = void>
struct A {
    static_assert(std::is_same_v<T, void>, "error");

    explicit A(B<T>& b)
        : _b(b)
    {}

    void fun() {
        _b.todo(*this);
    }

    B<T>& _b;
};

template <typename T = void>
struct B {
    void todo(A<T>& a) {
        static_cast<void>(a._b);
        HX::print::println("todo: B");
    }
};

#else

struct B;

struct A {
    explicit A(B& b)
        : _b(b)
    {}

    void func() {
        _b.todo(); // Member access into incomplete type 'B'
    }

    B& _b;
};

struct B {
    A& a;

    void todo() {}
};

#endif // ! #if
#endif // !__test_Forward_Declaration__

int main() {
#ifdef __test_Forward_Declaration__
    B b{};
    A a{b};
    a.fun();
#endif
    return 0;
}