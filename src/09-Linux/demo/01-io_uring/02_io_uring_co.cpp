#include <HXprint/print.h>

#include <cstring>
#include <chrono>
#include <coroutine>

#include <liburing.h>

// 资料
// https://github.com/axboe/liburing
// https://unixism.net/loti/
// https://man7.org/linux/man-pages/man7/io_uring.7.html

// https://cuterwrite.top/p/efficient-liburing/

namespace HX {

template <class Rep, class Period>
struct ::__kernel_timespec durationToKernelTimespec(std::chrono::duration<Rep, Period> dur) {
    struct ::__kernel_timespec ts;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs);
    ts.tv_sec = static_cast<__kernel_time64_t>(secs.count());
    ts.tv_nsec = static_cast<__kernel_time64_t>(nsecs.count());
    return ts;
}

struct IoUringErrorHandlingTools {
    inline static int check(int code) {
        if (code < 0) [[unlikely]] {
            throw std::system_error(-code, std::system_category());
        }
        return code;
    }
};

struct AioTask {
    AioTask(::io_uring_sqe* sqe) noexcept
        : _sqe{sqe}
    {
        ::io_uring_sqe_set_data(_sqe, this);
    }

    AioTask& operator=(AioTask const&) noexcept = delete;
    AioTask(AioTask const&) noexcept = delete;

    AioTask(AioTask&&) noexcept = default;
    AioTask& operator=(AioTask&&) noexcept = default;

    struct AioAwaiter {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr auto await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            _task->_previous = coroutine;
            _task->_res = -ENOSYS;
        }
        constexpr int await_resume() const noexcept {
            return _task->_res;
        }
        AioTask* _task;
    };

    AioAwaiter operator co_await() {
        return {this};
    }

private:
    friend struct IoUring;

    union {
        int _res;
        ::io_uring_sqe* _sqe;
    };
    std::coroutine_handle<> _previous;

public:
    // IO 操作 ...
};

struct IoUring {
    explicit IoUring(unsigned int size) noexcept
        : _ring{}
        , _numSqesPending{}
    {
        unsigned int flags = 0;

        IoUringErrorHandlingTools::check(
            ::io_uring_queue_init(size, &_ring, flags)
        );
    }

    ~IoUring() noexcept {
        ::io_uring_queue_exit(&_ring);
    }

    AioTask makeAioTask() {
        return AioTask{getSqe()};
    }

    void run(std::optional<std::chrono::system_clock::duration> timeout) {
        ::io_uring_cqe* cqe = nullptr;

        ::__kernel_timespec timespec; // 设置超时为无限阻塞
        ::__kernel_timespec* timespecPtr = nullptr;
        if (timeout.has_value()) {
            timespec = durationToKernelTimespec(*timeout);
            timespecPtr = &timespec;
        }

        // 阻塞等待内核, 返回是错误码; cqe是完成队列, 为传出参数
        int res = ::io_uring_submit_and_wait_timeout(
            &_ring, &cqe, 1, timespecPtr, nullptr);
        
        // 超时
        if (res == -ETIME) {
            // 内部直接 if (nr) { ... }, 此处调用传参 0, 毫无作用
            // ::io_uring_cq_advance(&_ring, 0); // 确保完成队列状态同步
            return;
        } else if (res < 0) [[unlikely]] { // 其他错误
            if (res == -EINTR) { // 被信号中断
                return;
            }
            throw std::system_error(-res, std::system_category());
        }

        unsigned head, numGot = 0;
        std::vector<std::coroutine_handle<>> tasks;
        io_uring_for_each_cqe(&_ring, head, cqe) {
            ++numGot;

            // @todo, 不能这样, 太多 if 了
            if (cqe->res < 0 
                && !(cqe->res == -ENOENT 
                    || cqe->res == -EACCES 
                    || cqe->res == -EAGAIN 
                    || cqe->res == -ECONNRESET 
                    || cqe->res == -EPIPE
                )
            ) {
                printf("Critical error: %s\n", strerror(-cqe->res));

                continue;
            }
            auto* task = reinterpret_cast<AioTask*>(cqe->user_data);
            task->_res = cqe->res;
            tasks.push_back(task->_previous);
        }

        // 手动前进完成队列的头部 (相当于批量io_uring_cqe_seen)
        ::io_uring_cq_advance(&_ring, numGot);
        _numSqesPending -= static_cast<std::size_t>(numGot);
        for (const auto& it : tasks) {
            it.resume();
        }
    }

private:
    ::io_uring_sqe* getSqe() {
        // 获取一个任务
        ::io_uring_sqe* sqe = ::io_uring_get_sqe(&_ring);
        while (!sqe) {
#if 0
            // 提交任务队列给内核 (为什么不是sqe, 因为sqe是从ring中get出来的, 故其本身就包含了sqe)
            int res = ::io_uring_submit(&_ring);
            if (res < 0) [[unlikely]] {
                if (res == -EINTR) {
                    continue;
                }
                throw std::system_error(-res, std::system_category());
            }
            // 提交了, 应该有空位, 那可以
            sqe = ::io_uring_get_sqe(&_ring);
#else
            ::io_uring_submit_and_wait(&_ring, 1); // 直接挂起等待操作系统完成了再说
            sqe = ::io_uring_get_sqe(&_ring);
            if (!sqe) {
                throw std::runtime_error("Still failed to get sqe after wait");
            }
#endif
        }
        ++_numSqesPending;
        return sqe;
    }

    ::io_uring _ring;
    std::size_t _numSqesPending; // 未完成的任务数
};

} // namespace HX

int main() {
    using namespace HX;
    return 0;
}
