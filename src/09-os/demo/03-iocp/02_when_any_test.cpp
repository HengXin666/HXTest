#include <HXprint/print.h>

#include <cstring>
#include <unordered_set>
#include <chrono>
#include <coroutine>
#include <thread>
#include <limits>

#include <coroutine/task/Task.hpp>
#include <coroutine/awaiter/WhenAny.hpp>
#include <coroutine/loop/TimerLoop.hpp>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <WinSock2.h>
#include <MSWSock.h>
#include <Windows.h>

// #pragma comment(lib, "Ws2_32.lib")
// #pragma comment(lib, "Mswsock.lib")

namespace HX {
    
void* checkWinError(void* data) {
    if (!data) {
        HX::print::println("Error: ", ::GetLastError());
        throw std::runtime_error{std::to_string(::GetLastError())};
    }
    return data;
}

DWORD toDwMilliseconds(std::chrono::system_clock::duration dur) {
    using namespace std::chrono;

    // 转换为毫秒（有符号 64 位整数）
    auto ms = duration_cast<milliseconds>(dur).count();

    // 如果负数或太大，就处理为 0 或 INFINITE
    if (ms <= 0)
        return 0;
    if (ms >= std::numeric_limits<DWORD>::max())
        return INFINITE;

    return static_cast<DWORD>(ms);
}

struct AioTask : public ::OVERLAPPED {
    enum class State {
        Normal,
        Cancel = 1,
    };

    AioTask(HANDLE iocpHandle, std::unordered_set<::HANDLE>& runingHandle) noexcept
        : _iocpHandle{iocpHandle}
        , _runingHandle{runingHandle}
    {
        ::memset(this, 0, sizeof(OVERLAPPED)); // 必须初始化 OVERLAPPED
        _state = State::Cancel; // 默认是取消
    }
#if 1 // 注意: 不能存在`移动`, 否则 IoUring::makeAioTask 返回就是 构造的新对象; 屏蔽了移动, 反而是编译器优化!
      // 得写移动, 不然无法在 MSVC 上通过编译 对于 HX::whenAny
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
        constexpr uint64_t await_resume() const noexcept {
            _task->_state = State::Normal;
            return _task->_res;
        }
        AioTask* _task;
    };

    AioAwaiter operator co_await() {
        return {this};
    }

private:
    friend struct Iocp;

    union {
        uint64_t _res;
        HANDLE _iocpHandle;
    };
    std::reference_wrapper<std::unordered_set<::HANDLE>> _runingHandle;
    std::coroutine_handle<> _previous;
    State _state; // 状态

    void associateHandle(HANDLE h) & {
        auto&& runingHandleRef = _runingHandle.get();
        if (runingHandleRef.count(h))
            return;
        if (!::CreateIoCompletionPort(
            h, _iocpHandle, (ULONG_PTR)h, 0) 
            && ::GetLastError() != ERROR_INVALID_PARAMETER
        ) {
            throw std::runtime_error{std::to_string(::GetLastError())};
        }
        runingHandleRef.insert(h);
    }

    struct _AioTimeoutTask {
        _AioTimeoutTask(AioTask&& self, TimerLoop::TimerAwaiter&& timerTask)
            : _self{std::make_unique<AioTask>(std::move(self))}
            , _timerTask{std::move(timerTask)}
        {
            print::println("__hx_func__");
        }

        _AioTimeoutTask(_AioTimeoutTask&&) = default;
        _AioTimeoutTask& operator=(_AioTimeoutTask&&) noexcept = default;

        Task<> co() {
            struct _del_print_ {
                _del_print_() {
                    print::println("_del_print_ begin {");
                }

                ~_del_print_() noexcept {
                    print::println("} // _del_print_ end");
                }
            } _;
            co_await _timerTask;
            _self->_state = State::Normal;
            co_return;
        }
        ~_AioTimeoutTask() noexcept {
            print::println("~__hx_func__");
        }
    private:
        friend AioTask;
        std::unique_ptr<AioTask> _self;
        TimerLoop::TimerAwaiter _timerTask;
    };
public:
    /**
     * @brief 异步读取文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param size 读取的长度
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRead(
    ) && {
        print::println("abc...");
        return std::move(*this);
    }

    /**
     * @brief 创建未链接的超时操作
     * @param ts 超时时间
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] _AioTimeoutTask prepLinkTimeout(
        TimerLoop::TimerAwaiter&& timerTask // 获得所有权, 此时 timerTask 生命周期由协程接管
    ) && {
        return {std::move(*this), std::move(timerTask)};
    }

    [[nodiscard]] inline static auto linkTimeout(
        AioTask&& task, 
        _AioTimeoutTask&& timeoutTask
    ) {
        // 为什么不能是捕获?
#if 0
        return [_task = std::move(task), 
                _timeoutTask = std::move(timeoutTask)]() mutable 
        -> Task<HX::AwaiterReturnValue<decltype(whenAny(std::move(task), timeoutTask.co()))>> {
            _timeoutTask._self->_iocpHandle = _task._iocpHandle;
            co_return co_await whenAny(std::move(_task), _timeoutTask.co());
        }();
#else
        return [](AioTask&& _task,  _AioTimeoutTask&& _timeoutTask) mutable 
        -> Task<HX::AwaiterReturnValue<decltype(whenAny(std::move(task), timeoutTask.co()))>> {
            _timeoutTask._self->_iocpHandle = _task._iocpHandle;
            co_return co_await whenAny(std::move(_task), _timeoutTask.co());
        }(std::move(task), std::move(timeoutTask));
#endif
    }

    ~AioTask() noexcept {
        HX::print::println(this, " 自杀了");
    }
};

struct Iocp {
    Iocp() 
        : _iocpHandle{checkWinError(::CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            nullptr,
            0,
            0))}
        , _runingHandle{}
        , _tasks{}
    {}

    Iocp& operator=(Iocp&&) = delete;

    AioTask makeAioTask() {
        return {_iocpHandle, _runingHandle};
    }

    bool isRun() const {
        return _runingHandle.size();
    }

    void run(std::optional<std::chrono::system_clock::duration>) {
        
    }

    ~Iocp() noexcept {
        if (_iocpHandle) {
            CloseHandle(_iocpHandle);
        }
    }

private:
    HANDLE _iocpHandle;
    std::unordered_set<::HANDLE> _runingHandle;
    std::vector<std::coroutine_handle<>> _tasks;
};

struct Loop {
    void start() {
        auto tasks = test1();
        static_cast<std::coroutine_handle<>>(tasks).resume();
        while (true) {
            auto timeout = _timerLoop.run();
            if (_iocp.isRun()) {
                _iocp.run(timeout);
            } else if (timeout) {
                std::this_thread::sleep_for(*timeout);
            } else {
                break;
            }
        }
    }
    
    auto makeTimer() {
        return TimerLoop::makeTimer(_timerLoop);
    }
private:
    // 控制台读写 (有问题... win的问题, 应该不是我的)
    Task<> test1() {
        using namespace HX;
        using namespace std::chrono;
        print::println("Task<> start!");

        while (true) {
            try {
                auto res = co_await AioTask::linkTimeout(
                    _iocp.makeAioTask().prepRead(),
                    _iocp.makeAioTask().prepLinkTimeout(
                        makeTimer().sleepFor(3s)
                    )
                );
                print::println("res: ", res.index());
                if (res.index() == 1) {
                    print::print("超时了~");
                    co_return;
                }
            } catch (std::exception& e) {
                std::cerr << "fxxk throw: " << e.what();
                co_return;
            }
        }
        co_return;
    }

    // 测试定时器
    Task<> test3() {
        using namespace std::chrono;
        print::println("等我 3s");
        co_await makeTimer().sleepFor(3s);
        print::println("等我 1s");
        co_await makeTimer().sleepFor(1s);
        print::println("我结束啦, 但是还要等一下...");
        co_await whenAny(
            makeTimer().sleepFor(1s), 
            makeTimer().sleepFor(120s)
        );
        print::println("才等了 1s...");
    }

    Iocp _iocp;
    TimerLoop _timerLoop;
};

} // namespace HX

using namespace std::chrono;

int main() {
    using namespace HX;
    setlocale(LC_ALL, "zh_CN.UTF-8");
    Loop loop;
    loop.start();
    // constexpr bool _ = AwaitableLike<Task<>>; 
    return 0;
}