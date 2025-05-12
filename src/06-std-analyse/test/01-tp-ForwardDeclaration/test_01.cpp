#include <HXprint/print.h>

// 前向声明 & 模版名称二阶段查找
#define __test_Forward_Declaration__

#ifdef __test_Forward_Declaration__

template <typename T>
struct A;

struct B {
    template <typename T>
    auto func(A<T> const& a) {
        a.func();
        return a.data;
    }

    template <typename T, typename = std::enable_if_t<!std::is_same_v<T, void>>>
    auto func(A<T> const& a) {
        // 恒成立, 无所谓, 不会触发的
        static_assert(sizeof(T) > 0, "error");
    }
};

template <typename T = void>
struct A {
    int data;

    void func() const {
        HX::print::println("A func");
    }
};

#else

struct A;

struct B {
    auto func(A const& a) {
        // Member access into incomplete type 'const A'
        // 成员访问不完整类型 "const A"
        a.func();
        return a.data;
    }
};

struct A {
    int data;

    void func() const {
        HX::print::println("A func");
    }
};

#endif

int main() {
    auto res1 = B{}.func(A{});
    static_cast<void>(res1);

    // auto res2 = B{}.func(A<int>{});
    // static_cast<void>(res2);
    return 0;
}