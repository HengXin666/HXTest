#include <HXprint/print.h>
#include <memory>

struct Base {
    ~Base() noexcept {
        HX::print::println("~Base");
    }
};

struct Boy : public Base {
    // Boy() { HX::print::println("Boy"); }
    ~Boy() noexcept {
        HX::print::println("~Boy");
    }
};

// === 下面使用了虚析构 ===

struct VBase {
    virtual ~VBase() noexcept {
        HX::print::println("~VBase");
    }
};

struct VBoy : public VBase {
    ~VBoy() noexcept override {
        HX::print::println("~VBoy");
    }
};

/**
 * @brief 结论:
 * 虚析构, 只有在使用 基类指针 = new 子类
 * 并且 del 基类指针 时候才需要虚析构
 *
 * 因为不这样, 基类指针实际上不知道有运行时多态, 而多了 virtual
 * 基类指针就知道要做什么了 (编译器实现)
 */

int main() {
    // 修改这里, 运行测试
    switch (13) {
        case 1: [](){
            Base base;
        }(); break;
        case 2: [](){
            Boy boy;
        }(); break;
        case 3: [](){
            Base* p = new Base;
            delete p;
        }(); break;
        case 4: [](){
            Boy* p = new Boy;
            delete p;
        }(); break;
        // === 上面的 1~4 都是正常的, 可以完全释放的 (很明显的) ===
        case 5: [/* !!! */](){
            Base* p = new Boy; // 只会调用 ~Base 【有内存泄漏风险】
            delete p;
        }(); break;
        case 6: [](){
            HX::print::println("---RUN--- {");
            Base const& r = Boy{}; // 会调用 ~Base 和 ~Boy
            static_cast<void>(r);
            HX::print::println("} // ---END---");
        }(); break;
        case 7: [](){
            // === 下面是安全的 ===
            VBase* p = new VBoy; // 会调用 ~VBase 和 ~VBoy
            delete p;
        }(); break;
        case 8: [](){
            // === 下面是安全的 ===
            Base* p = new Boy;              // 会调用 ~Base 和 ~Boy
            delete static_cast<Boy*>(p);    // 这样是ok的, 因为运行时候默认是基于类型查找对应的函数的
        }(); break;
        case 9: [/* !!! */](){
            Boy* p = new Boy;
            delete static_cast<Base*>(p);   // 同理, 此处仅会调用 ~Base
        }(); break;
        case 10: [](){
            VBoy* p = new VBoy;
            delete static_cast<VBase*>(p);  // 会调用 ~VBase 和 ~VBoy (安全的)
        }(); break;
        // === 下面是对于智能指针的实验 ===
        case 11: [/* !!! */](){
            std::unique_ptr<Base> p = std::make_unique<Boy>();
            // 此处仅会调用 ~Base
            // 内存泄漏了! 为什么呢? 可以取消 Boy 的构造函数的注释, 你会发现, 它真的被构造了!
            // 底层原因是 std::make_unique<Boy>() 等价于 new Boy
            // std::unique_ptr<Base> p = 
            // 实际上是 template <typename U> unique_ptr(U* ptr) : _ptr(ptr) {}
            // 也就是说 p 实际上被强转为 U (也就是 Base) 类型了
            // 而 ~unique_ptr 析构实际上是 delete _ptr
            // 而此时的 _ptr 类型是 U (Base), 也就是上面的情况 [5]
        }(); break;
        case 12: [](){
            std::unique_ptr<VBase> p = std::make_unique<VBoy>();  // 会调用 ~Base 和 ~Boy (安全的)
            // 此处就同理是 [7]
        }(); break;
        case 13: [](){
            // === 对于 [11], 这里只是修改为 shared_ptr, 为什么这里是安全的??? ===
            std::shared_ptr<Base> p 
                // = std::make_unique<Boy>(); // 注意哦, 实际上是可以这样的, 因为 std::shared_ptr 可以从 unique 构造
                   = std::make_shared<Boy>();
            // 底层原因是, shared_ptr 它内部是类型擦除的, 因此可以调用到原本的 Boy 类型析构
            // 而不是 当时的(Base) 类型的析构
        }(); break;
    }
    return 0;
}

template <typename T>
struct UPtr {
    template <typename U>
    UPtr(U* ptr) : _ptr(ptr) {}
    
    ~UPtr() noexcept {
        delete /*static_cast<T*>(*/ _ptr /*)*/;
    }

private:
    T* _ptr{};
};

namespace internal {

struct TypeFkBase {
    virtual void del() = 0;
    virtual ~TypeFkBase() noexcept {}
};

template <typename T>
struct TypeFk : public TypeFkBase {
    TypeFk(T* _p) : p(_p) {};
    ~TypeFk() noexcept override {}
    void del() override {
        delete p;
    }
    T* p;
};

} // namespace internal

template <typename T>
struct SPtr {
    template <typename U>
    SPtr(U* ptr) 
        : _ptr(new internal::TypeFk<U>{ptr})
    {}

    ~SPtr() noexcept {
        _ptr->del();
        delete _ptr;
    }
private:
    internal::TypeFkBase* _ptr;
};

/**
 * @brief 测试自己实现的伪智能指针 (U / S) 系列对于析构的掌握
 */
auto __init__ = []{
    if constexpr (false)
        return 0;
    HX::print::println("U Ptr {");
    {
        UPtr<Base> p{new Boy};
    }
    HX::print::println("} // U Ptr\n");
    HX::print::println("S Ptr {");
    {
        SPtr<Base> p{new Boy};
    }
    HX::print::println("} // S Ptr\n");
    return 0;
}();