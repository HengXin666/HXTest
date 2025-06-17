#include <HXprint/print.h>

#include <cstring>
#include <chrono>
#include <coroutine>
#include <thread>

#include <coroutine/task/Task.hpp>
#include <coroutine/awaiter/WhenAny.hpp>

// #include <WinSock2.h>
// #include <MSWSock.h>

#define WIN32_LEAN_AND_MEAN
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

struct AioTask : public ::OVERLAPPED {
    AioTask(HANDLE iocpHandle) noexcept
        : _iocpHandle{iocpHandle}
    {
        ::memset(this, 0, sizeof(OVERLAPPED)); // 必须初始化 OVERLAPPED
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
    friend struct Iocp;

    union {
        int _res;
        HANDLE _iocpHandle;
    };
    std::coroutine_handle<> _previous;

    void associateHandle(HANDLE h) & {
        checkWinError(::CreateIoCompletionPort(
            h, _iocpHandle, (ULONG_PTR)h, 0));
    }
public:
    // IO 操作 ...
    /**
     * @brief 异步读取文件
     * @param fd 文件句柄
     * @param buf [out] 读取到的数据
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRead(
        HANDLE fd,
        std::span<char> buf,
        std::uint64_t offset
    ) && {
        // ::io_uring_prep_read(_sqe, fd, buf.data(), static_cast<unsigned int>(buf.size()), offset);
        /*
BOOL ReadFile(
  HANDLE       hFile,                // 文件句柄（可为文件、管道、串口、Socket 等）
  LPVOID       lpBuffer,            // 数据读入的缓冲区指针（你准备好的内存）
  DWORD        nNumberOfBytesToRead,// 想要读取的字节数
  LPDWORD      lpNumberOfBytesRead, // 实际读取的字节数（同步时非 NULL，异步时设为 NULL）
  LPOVERLAPPED lpOverlapped         // OVERLAPPED 结构指针（异步时必填，同步时为 NULL）
);
        */
        /*
typedef struct _OVERLAPPED {
  ULONG_PTR Internal;
  ULONG_PTR InternalHigh;
  union {
    struct {
      DWORD Offset;      // 低 32 位文件偏移量
      DWORD OffsetHigh;  // 高 32 位文件偏移量
                         // 两个组成 64 为的偏移量, 破win为了远古兼容, 就这样搞了...
    };
    PVOID Pointer;
  };
  HANDLE    hEvent;
} OVERLAPPED, *LPOVERLAPPED;
        */
        HX::print::println("add {");
        associateHandle(fd);
        HX::print::println("} // add");
        // 设置偏移量
        Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
        OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);
        bool ok = ::ReadFile(
            fd,
            buf.data(),
            static_cast<DWORD>(buf.size()),
            nullptr,
            static_cast<::OVERLAPPED*>(this)
        );
        if (!ok) [[unlikely]] {
            DWORD err = ::GetLastError();
            if (err != ERROR_IO_PENDING) {
                std::cerr << "ReadFile failed: " << err << '\n';
                std::terminate();
            }
        }
        return std::move(*this);
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
        , _numSqesPending{}
        , _tasks{}
    {}

    Iocp& operator=(Iocp&&) = delete;

    AioTask makeAioTask() {
        ++_numSqesPending;
        return {_iocpHandle};
    }

    bool isRun() const {
        return _numSqesPending;
    }

    void run() {
/*
// 只能一次获取一个
BOOL GetQueuedCompletionStatus(
    [in]  HANDLE       CompletionPort,             // 完成端口的句柄
    [out] LPDWORD      lpNumberOfBytesTransferred, // 指针: 接收完成的I/O操作传输的字节数
    [out] PULONG_PTR   lpCompletionKey,            // 指针: 接收与完成的I/O操作关联的完成键
    [out] LPOVERLAPPED *lpOverlapped,              // 指针: 接收指向完成的I/O操作的OVERLAPPED结构的指针
    [in]  DWORD        dwMilliseconds              // 等待操作完成的超时时间, 以毫秒为单位
);

// 一次获取多个
BOOL GetQueuedCompletionStatusEx(
  HANDLE               CompletionPort,          // 完成端口的句柄
  LPOVERLAPPED_ENTRY   lpCompletionPortEntries, // 批量接收的数组
  ULONG                ulCount,                 // 最大获取个数
  PULONG               ulNumEntriesRemoved,     // 实际返回个数
  DWORD                dwMilliseconds,          // 等待操作完成的超时时间, 以毫秒为单位
  BOOL                 fAlertable               // 是否可被 APC (异步过程调用) 或 IO Completion Callback 中断
                                                // 一般写 false, 不允许!
);
*/
        std::array<::OVERLAPPED_ENTRY, 64> arr;
        ULONG n;

        ::GetQueuedCompletionStatusEx(
            _iocpHandle,
            arr.data(),
            static_cast<DWORD>(arr.size()),
            &n,
            INFINITE,
            false
        );

        for (ULONG i = 0; i < n; ++i) {
            auto ptr = arr[i];
            auto task = reinterpret_cast<AioTask*>(ptr.lpOverlapped);
            task->_res = ptr.dwNumberOfBytesTransferred;
            _tasks.push_back(task->_previous);
        }

        for (const auto& t : _tasks) {
            t.resume();
        }

        _numSqesPending -= n;
        _tasks.clear();
    }

    ~Iocp() noexcept {
        if (_iocpHandle) {
            CloseHandle(_iocpHandle);
        }
    }

private:
    HANDLE _iocpHandle;
    std::size_t _numSqesPending;
    std::vector<std::coroutine_handle<>> _tasks;
};

struct Loop {
    void start() {
        auto tasks = task();
        static_cast<std::coroutine_handle<>>(tasks).resume();
        while (_iocp.isRun()) {
            _iocp.run();
        }
    }
    
private:
    Task<> task() {
        using namespace HX;
        using namespace std::chrono;
        print::println("Task<> start!");

        // iocp 不太能 读控制台流...
        HANDLE hStdin = CreateFile(
            "CONIN$", 
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
        );
        
        if (hStdin == INVALID_HANDLE_VALUE) {
            DWORD err = ::GetLastError();
            std::cerr << "CreateFile(CONIN$) failed: " << err << '\n';
            co_return;
        }
        
        // 设置控制台为行输入模式
        DWORD mode = 0;
        if (!GetConsoleMode(hStdin, &mode)) {
            std::cerr << "GetConsoleMode failed\n";
            CloseHandle(hStdin);
            co_return;
        }
        
        // mode |= ENABLE_LINE_INPUT;   // 行输入模式
        // mode |= ENABLE_ECHO_INPUT;    // 回显输入
        // mode |= ENABLE_PROCESSED_INPUT; // 处理控制键
        
        // if (!SetConsoleMode(hStdin, mode)) {
        //     std::cerr << "SetConsoleMode failed\n";
        //     CloseHandle(hStdin);
        //     co_return;
        // }
        while (true) {
            std::string buf;
            buf.resize(128);
            std::cerr << "cin >> ";
            try {
                co_await _iocp.makeAioTask().prepRead(hStdin, buf, 0);
            } catch (std::exception& e) {
                std::cerr << "fxxk throw: " << e.what();
                co_return;
            }
            print::println("echo: ", buf);
        }
        co_return;
    }

    Iocp _iocp;
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

int main() {
    using namespace HX;
    setlocale(LC_ALL, "zh_CN.UTF-8");
    Loop loop;
    loop.start();
    return 0;
}