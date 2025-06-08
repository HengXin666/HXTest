#include <HXprint/print.h>

#include <liburing.h>

// 资料
// https://github.com/axboe/liburing
// https://unixism.net/loti/
// https://man7.org/linux/man-pages/man7/io_uring.7.html

// https://cuterwrite.top/p/efficient-liburing/

namespace HX {

struct IoUring {
    explicit IoUring(unsigned int size) noexcept
        : _ring{}
    {
        ::io_uring_queue_init(size, &_ring, 0);
    }

    ~IoUring() noexcept {
        ::io_uring_queue_exit(&_ring);
    }


private:
    ::io_uring _ring;
};

} // namespace HX


int main() {
    HX::print::println("Hello Io_uring");
    return 0;
}
