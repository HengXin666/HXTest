#include <HXprint/print.h>

// #define __test_Forward_Declaration__

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
    B<T> b;
};

template <typename T = void>
struct B {
    A<T> a; // 字段的类型 "A<void>" 不完整
            // Field has incomplete type 'A<void>'
};

#else

struct B;

struct A {
    B b; // Field has incomplete type 'B'
};

struct B {
    A a;
};

#endif // ! #if
#endif // !__test_Forward_Declaration__

int main() {
#ifdef __test_Forward_Declaration__
    A a{};
    B b{};
#endif
    return 0;
}