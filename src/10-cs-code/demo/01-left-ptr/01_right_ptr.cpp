#include <HXTest.hpp>

struct RightPtr {
    auto operator->() noexcept {
        return this;
    }
    int data_1;
};

struct RightObjPtr {
    auto operator->() noexcept {
        return RightPtr{};
    }
    int data_2;
};

struct RightRef {
    auto& operator->() noexcept {
        return *this;
    }
    int data_3;
};

HX_NO_WARNINGS_BEGIN
int main() {
    RightPtr p;
    p->operator->()->operator->()->operator->()->operator->()->data_1;

    RightObjPtr objPtr;
    objPtr->data_1; // 递归的调用返回的对象的 ->, 知道找到一个 指针

    RightRef ref;
    // ref-> // Circular pointer delegation detected
             // -> 要求需要返回为指针 或者 obj 同样具有 -> 运算符, 以进行递归调用, 直到为指针
    
    // is ok
    RightPtr* pp = new RightPtr;
    pp->operator->()->data_1;
    delete pp;
    return 0;
}
HX_NO_WARNINGS_END