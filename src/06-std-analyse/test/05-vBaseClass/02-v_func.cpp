#include <HXTest.hpp>

struct Base {
    void func() {
        log::hxLog.info("Base-func");
    }

    ~Base() noexcept {
        log::hxLog.info("~Base");
    }
};

struct Sub : Base {
    void func() {
        log::hxLog.warning("Sub-func");
    }

    ~Sub() noexcept {
        log::hxLog.warning("~Sub");
    }
};

int main() {
    // 基于引用或对象类型的静态绑定 (compile-time polymorphism 的一种特殊形式)
    // 勉强算 编译期多态 ~~
    Base&& obj = Sub{};
    obj.func();
}