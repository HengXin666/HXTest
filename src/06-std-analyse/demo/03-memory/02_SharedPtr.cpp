#include <HXprint/print.h>

#include <atomic>

namespace hx {

namespace internal {

template <typename T>
struct AtomicCnt {
    void add() {
        ++_cnt;
    }

    bool minus() {
        uint64_t cnt = --_cnt;
        return cnt == 0;
    }
private:
    T* _data{nullptr};
    std::atomic_uint64_t _cnt{1};
};

} // namespace internal

template <typename T>
struct SharedPtr {
    explicit SharedPtr() noexcept {}

    ~SharedPtr() noexcept {
        
    }
private:
    internal::AtomicCnt<T>* _ptr{nullptr};
};

} // namespace hx

int main() {

    return 0;
}