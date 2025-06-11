#include <HXprint/print.h>

/**
 * @brief 本篇毫无营养, 请不要对号入座!
 */

struct Awa {
    explicit Awa() {
        HX::print::println("A构造");
    }

    explicit Awa(int) {
        throw std::runtime_error{""};
    }

    ~Awa() noexcept {
        HX::print::println("~A()");
    }
};

void fun(Awa*, Awa*) {
}

int main() {
    HX::print::println("修改前:");
    try {
        // 今天在小红书上看见了一个喷C++异常不好用的,
        // 说什么栈解旋, 扯什么有时候不会释放的 (感觉就是懂个名词的傻铲)
        // 感觉就是个菜逼, RAII都不懂
        // 下面是它举例的:
        // A 成功构造, 而 B 构造抛出了异常, 说不会析构A
        // 下面是示例代码 
        fun(new Awa(), new Awa(int{}));
    } catch (...) {
        // 评价: 你的指针会无缘无故析构对吗?
        // 再者: C++说了不要在构造和析构中使用异常, 你以为你写C++98的就是985的啊
    }

    // 修改: 使用RAII
    HX::print::println("\n修改后:");
    try {
        Awa a1{};
        Awa a2{int{}};
        fun(&a1, &a2);
    } catch (...) {
        // 懒得喷
    }
    return 0;
}