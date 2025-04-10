#include <HXprint/print.h>

// @todo 这个只是简易的 UniquePtr 智能指针, 还有些细节没有实现

namespace HX {

template <typename T>
struct UniquePtr {
    explicit UniquePtr() noexcept
        : _data()
    {}

    explicit UniquePtr(T* data) noexcept
        : _data(data)
    {}

    explicit UniquePtr(UniquePtr const&) noexcept = delete;
    UniquePtr& operator=(UniquePtr const&) = delete;

    T* operator->() {
        return _data;
    }

    T* get() const noexcept {
        return _data;
    }

    ~UniquePtr() noexcept {
        if (_data) {
            delete _data;
        }
    }

private:
    T* _data;
};

template <typename T, typename... Ts>
UniquePtr<T> makeUniquePtr(Ts&&... ts) noexcept {
    return UniquePtr<T>(new T{ts...});
}

} // namespace HX


struct RaiiPrint {
    template <typename... Ts>
    RaiiPrint(Ts&&...) {
        HX::print::println("构造: {");
    }

    void show() {
        HX::print::println("show \\awa/");
    }

    ~RaiiPrint() {
        HX::print::println("} // 析构");
    }
};

int main() {
    auto ptr = HX::makeUniquePtr<RaiiPrint>(1, 2, 3, HX::makeUniquePtr<RaiiPrint>(1, 2, 3));
    ptr->show();
    return 0;
}