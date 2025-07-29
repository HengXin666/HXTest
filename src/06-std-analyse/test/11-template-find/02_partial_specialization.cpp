#include <HXprint/print.h>
using namespace HX;
#include <vector>

template <typename T>
struct A {
    static void func(int cnt = 0) {
        if (cnt > 2)
            return;
        print::println("主模板");
        // A<std::vector<int>>::func(++cnt);

        // 需要依赖了才会触发
        // Type _;
    }
    using Type = A<std::vector<int>>;
};

// template <typename T>
// struct _awa_qwq_ {
//     using Type = std::vector<int>;
//     void __func() {
//         A<Type>::func();
//     }
// };

auto __test_01__ = []{
    // A<std::vector<int>>::func();
    return 0;
}();

template <typename T>
struct A<std::vector<T>> {
    static void func(int cnt = 0) {
        if (cnt > 2)
            return;
        print::println("A<int>特化, 模板");
    }
};

auto __test_02__ = []{
    A<std::vector<int>>::func();
    return 0;
}();

int main() {
    A<std::vector<int>>::func();
    return 0;
}