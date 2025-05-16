#include <HXprint/print.h>

/**
 * @brief 奇异递归模版 (CRTF)
 * 用于实现静态多态
 */

template <typename T>
struct Man {
    void oi() {
        static_cast<T*>(this)->oi();
    }

    void kawaii() {
        static_cast<T*>(this)->kawaii();
    }

    // 链式调用
    T& zzz() {
        HX::print::print("zzz... -> ");
        return static_cast<T&>(*this);
    }
};

struct LoLi : public Man<LoLi> {
    void oi() {
        HX::print::println("LoLi: o... o ha yo~");
    }

private:
    friend struct Man<LoLi>; // 如果基类访问的不是公有的, 那么需要声明为友元

    void kawaii() {
        HX::print::println("LoLi: hei hei hei~");
    }
};

struct Imouto : public Man<Imouto> {
    void oi() {
        HX::print::println("Imouto: o ha yo~ o ni tya~");
    }

private:
    friend struct Man<Imouto>;

    void kawaii() {
        HX::print::println("Imouto: mu kyu~");
    }
};

/**
 * @brief 使用示例: 静态多态
 * @tparam T 
 * @param man 
 */
template <typename T>
void greet(Man<T>& man) {
    man.zzz().oi();
    man.kawaii();
}

int main() {
    LoLi loli{};
    Imouto imouto{};
    greet(loli);
    greet(imouto);
    return 0;
}