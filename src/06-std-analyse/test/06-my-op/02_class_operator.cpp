#include <HXTest.hpp>

template <typename Base, typename T>
bool constexpr isBaseMan = std::is_same_v<Base, T> || std::is_base_of_v<Base, T>;

template <typename T>
struct SPtr {
    SPtr() = default;

    template <typename U>
    SPtr(SPtr<U> const& that) noexcept
        : _data{that._data}
    {}

    template <typename U>
    SPtr(SPtr<U>&& that) noexcept
        : _data{that._data}
    {
        that._data = nullptr;
    }

    ~SPtr() noexcept {
        if (_data) {
            delete _data;
        }
    }
private:
    template <typename>
    friend struct SPtr;

    T* _data{nullptr};
};

class A{};

class B : public A {};

class C : public B {};

int main() {
    SPtr<B> ptr{};
    [[maybe_unused]] auto ptr_2 = ptr;
    [[maybe_unused]] auto ptr_3 = std::move(ptr);

    [[maybe_unused]] auto ptr_4 = SPtr<A>{ptr};
    [[maybe_unused]] auto ptr_5 = SPtr<A>{std::move(ptr)};
}