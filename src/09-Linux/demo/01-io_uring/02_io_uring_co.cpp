#include <HXprint/print.h>

#include <cstring>
#include <chrono>
#include <coroutine>
#include <thread>

#include <liburing.h>

// 资料
// https://github.com/axboe/liburing
// https://unixism.net/loti/
// https://man7.org/linux/man-pages/man7/io_uring.7.html

// https://github.com/0voice/kernel_new_features
// https://cuterwrite.top/p/efficient-liburing/

#include <coroutine/task/Task.hpp>
#include <coroutine/awaiter/WhenAny.hpp>

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

#if 0 // 注意: 不能存在`移动`, 否则 IoUring::makeAioTask 返回就是 构造的新对象; 屏蔽了移动, 反而是编译器优化!
    AioTask& operator=(AioTask const&) noexcept = delete;
    AioTask(AioTask const&) noexcept = delete;

    AioTask(AioTask&&) noexcept = default;
    AioTask& operator=(AioTask&&) noexcept = default;
#else
    AioTask& operator=(AioTask&&) noexcept = delete;
#endif

    struct AioAwaiter {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_suspend(std::coroutine_handle<> coroutine) const noexcept {
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
    /**
     * @brief 异步读取文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRead(
        int fd,
        std::span<char> buf,
        std::uint64_t offset
    ) && {
        ::io_uring_prep_read(_sqe, fd, buf.data(), static_cast<unsigned int>(buf.size()), offset);
        return std::move(*this);
    }

    /**
     * @brief 创建未链接的超时操作
     * @param ts 超时时间
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepLinkTimeout(
        struct __kernel_timespec *ts,
        unsigned int flags
    ) && {
        ::io_uring_prep_link_timeout(_sqe, ts, flags);
        return std::move(*this);
    }

    [[nodiscard]] inline static auto linkTimeout(AioTask&& lhs, AioTask&& rhs) {
        lhs._sqe->flags |= IOSQE_IO_LINK;
        return whenAny(std::move(lhs), std::move(rhs));
    }

    ~AioTask() noexcept {
        HX::print::println(this, " 自杀了");
    }
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
        // mandatory copy elision 场景
        // 编译器强制使用 RVO (返回值优化)
        // https://en.cppreference.com/w/cpp/language/copy_elision.html
        return AioTask{getSqe()};
    }

    bool isRun() const noexcept {
        return _numSqesPending;
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
        io_uring_for_each_cqe(&_ring, head, cqe) {
            ++numGot;

            // @todo, 不能这样, 太多 if 了
            if (cqe->res < 0
                // && !(cqe->res == -ENOENT        // 文件或目录不存在（可能是非致命错误，例如取消已完成请求）
                //     || cqe->res == -EACCES      // 权限被拒绝（比如尝试访问受限文件）
                //     || cqe->res == -EAGAIN      // 资源暂时不可用（一般是非阻塞IO导致的，可重试）
                //     || cqe->res == -ECONNRESET  // 连接被对方重置（网络编程中常见）
                //     || cqe->res == -EPIPE       // 管道破裂（例如写入已关闭的 socket）
                // )
            ) {
                printf("Critical error: %s\n", strerror(-cqe->res));
                if (cqe->res == -ECANCELED) { // 操作已取消 (比如超时了)
                    continue;
                }
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

        tasks.clear();
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
    std::vector<std::coroutine_handle<>> tasks; // 协程任务队列
                                                // 提取为成员, 避免频繁构造临时变量导致频繁扩容
};

struct Loop {
    void start() {
        auto tasks = task();
        static_cast<std::coroutine_handle<>>(tasks).resume();
        while (ioUring.isRun()) {
            ioUring.run({});
        }
    }
    
private:
    Task<> task() {
        using namespace HX;
        using namespace std::chrono;
        auto kt = durationToKernelTimespec(3s);
        print::println("Task<> start!");
        while (true) {
            std::string buf;
            buf.resize(128);
            std::cerr << "cin >> ";
            auto res = co_await AioTask::linkTimeout(
                ioUring.makeAioTask().prepRead(STDIN_FILENO, buf, 0),
                ioUring.makeAioTask().prepLinkTimeout(&kt, 0)
            );
            std::cout << res.index() << "] ";

            if (res.index() == 1) {
                print::println("时间到了哦~");
                break;
            }
            
            // co_await ioUring.makeAioTask().prepRead(STDIN_FILENO, buf, 0);
            if (auto pos = buf.find('\n'); pos != std::string::npos) {
                buf[pos] = '\0';
            }
            if (buf.find("exit") != std::string::npos) [[unlikely]] {
                break;
            }
            print::println("echo: ", buf);
        }
        co_return;
    }

    IoUring ioUring{128};
};

} // namespace HX

/*
    模型

    task(); // 启动任务, 等任务挂起了
    loop(); ->  while (io_uring.isRun()) {
                    io_uring.run(); // 采用中断, 恢复协程
                }
*/

using namespace std::chrono;

HX::Task<> func02() {
    HX::print::println("func02 ---");
    // std::this_thread::sleep_for(2s);
    HX::print::println("func02 ---");
    co_return;
}

HX::Task<> func03() {
    HX::print::println("func03 ---");
    // std::this_thread::sleep_for(3s);
    HX::print::println("func03 ---");
    co_return;
}


HX::Task<> func01() {
    HX::print::println("func01 {");
    co_await HX::whenAny(
        func02(),
        []() -> HX::Task<> { co_return; }(), 
        func03()
    );
    HX::print::println("} // func01");
    co_return;
}

#include <variant>

int main() {
    using namespace HX;
    setlocale(LC_ALL, "zh_CN.UTF-8");
    Loop loop;
    loop.start();
    // static_cast<std::coroutine_handle<>>(func01())();
    return 0;
}
