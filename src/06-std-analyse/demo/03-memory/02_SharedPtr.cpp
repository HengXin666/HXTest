#include <HXprint/print.h>

#include <atomic>

namespace hx {

namespace internal {

struct AtomicCnt {
    std::size_t cnt() const {
        return _cnt;
    }

    void add() {
        ++_cnt;
    }

    /**
     * @brief 原子的自减后, 值是否为 0
     * @return true 
     * @return false 
     */
    bool minusIsZeRo() {
        uint64_t cnt = --_cnt;
        return cnt == 0;
    }
private:
    std::atomic_uint64_t _cnt{1};
};

} // namespace internal

template <typename T>
struct SharedPtr {
    explicit SharedPtr() noexcept 
        : _cnt(new internal::AtomicCnt{})
    {}

    explicit SharedPtr(std::nullptr_t) noexcept // 支持 nullptr 构造
        : _cnt(new internal::AtomicCnt{})
    {}

    explicit SharedPtr(T* data) noexcept
        : _cnt(new internal::AtomicCnt{})
        , _ptr(data)
    {}

    SharedPtr(SharedPtr const& that) noexcept {
        _cnt = that._cnt;
        _ptr = that._ptr;
        _cnt->add();
    }

    SharedPtr(SharedPtr&& that) noexcept {
        _cnt = that._cnt;
        _ptr = that._ptr;
        that._cnt = nullptr;
        that._ptr = nullptr;
    }

    SharedPtr& operator=(SharedPtr const& that) noexcept {
        if (this == &that)
            return *this;
        doReset();
        _cnt = that._cnt;
        _ptr = that._ptr;
        _cnt->add();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& that) noexcept {
        if (this == &that)
            return *this;
        doReset();
        _cnt = that._cnt;
        _ptr = that._ptr;
        that._cnt = nullptr;
        that._ptr = nullptr;
        return *this;
    }

    std::size_t useCount() const {
        return _cnt->cnt();
    }

    T* operator->() noexcept {
        return _ptr;
    }

    T& operator*() noexcept {
        return _ptr;
    }

    operator bool() noexcept {
        return _ptr;
    }

    T* get() const {
        return _ptr;
    }

    void reset() noexcept {
        doReset();
        _cnt = new internal::AtomicCnt{}; // 不能再和之前的共享计数了
        _ptr = nullptr;
    }

    void reset(T* data) noexcept {
        doReset();
        _cnt = new internal::AtomicCnt{};
        _ptr = data;
    }

    void swap(SharedPtr& that) noexcept {
        std::swap(_cnt, that._cnt);
        std::swap(_ptr, that._ptr);
    }

    ~SharedPtr() noexcept {
        doReset();
    }
private:
    void doReset() noexcept {
        if (_cnt->minusIsZeRo()) {
            delete _cnt;
            delete _ptr;
        }
    }

    internal::AtomicCnt* _cnt{nullptr};
    T* _ptr{nullptr};
};

template <typename T, typename... Ts>
SharedPtr<T> makeSharedPtr(Ts&&... ts) {
    return SharedPtr<T>{new T{std::forward<Ts>(ts)...}};
}

} // namespace hx

struct Msvc {
    std::string code;
    int cppV;

    ~Msvc() {
        HX::print::println("~Msvc");
    }
};

int main() {
    auto sp = hx::makeSharedPtr<Msvc>("锟斤拷", 17);
    {
        auto spp = sp;
        if (spp) {
            spp.reset();
            if (!spp) {
                HX::print::println("good!");
            }
        }
    }
    return 0;
}