#include <HXprint/print.h>

#include <cstring>
#include <unordered_set>
#include <chrono>
#include <coroutine>
#include <thread>
#include <limits>

#include <coroutine/task/Task.hpp>
#include <coroutine/task/RootTask.hpp>
#include <coroutine/awaiter/WhenAny.hpp>
#include <coroutine/loop/TimerLoop.hpp>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <MSWSock.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

namespace HX {

namespace internal {

struct InitWin32Api {
    InitWin32Api() {
        WSADATA data;
        if (::WSAStartup(MAKEWORD(2, 2), &data)) {
             throw std::runtime_error{"WSAStartup ERROR: " + std::to_string(::GetLastError())};
        }
    }

    ~InitWin32Api() noexcept {
        ::WSACleanup();
    }
} __initWin32Api__;

} // namespace internal

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
        ::HANDLE _iocpHandle;
    };
    std::reference_wrapper<std::unordered_set<::HANDLE>> _runingHandle;
    std::coroutine_handle<> _previous;

    void associateHandle(::HANDLE h) & {
        auto&& runingHandleRef = _runingHandle.get();
        if (runingHandleRef.count(h))
            return;
        if (!::CreateIoCompletionPort(
            h, _iocpHandle, static_cast<ULONG_PTR>(State::Normal), 0) 
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
        {}

        _AioTimeoutTask(_AioTimeoutTask&&) = default;
        _AioTimeoutTask& operator=(_AioTimeoutTask&&) noexcept = default;

        Task<> co() {
            co_await _timerTask;
            /*
BOOL PostQueuedCompletionStatus(
    HANDLE       CompletionPort,                // 目标完成端口的句柄
    DWORD        dwNumberOfBytesTransferred,    // 自定义的字节数, 可用于传递信息
    ULONG_PTR    dwCompletionKey,               // 自定义的完成键, 可用于区分不同的操作或I/O源
    LPOVERLAPPED lpOverlapped                   // OVERLAPPED结构的指针
        // 注: GetQueuedCompletionStatusEx 拿到的是 此处的 OVERLAPPED
        // 但是之前的 OVERLAPPED 也会被 iocp 取出! 因此我们需要自己标记一下 ... 写个状态机 ...
);
            */
            bool ok = ::PostQueuedCompletionStatus(
                _self->_iocpHandle,
                0,
                static_cast<ULONG_PTR>(State::Cancel),
                static_cast<::OVERLAPPED*>(_self.get())
            );
            if (!ok) [[unlikely]] {
                throw std::runtime_error{"PostQueuedCompletionStatus ERROR: " 
                    + std::to_string(::GetLastError())};
            }
            co_return;
        }
    private:
        friend AioTask;
        std::unique_ptr<AioTask> _self;
        TimerLoop::TimerAwaiter _timerTask;
    };
public:
    // IO 操作 ...
    // https://learn.microsoft.com/zh-cn/windows/win32/fileio/i-o-completion-ports

/*
支持的 I/O 函数

AcceptEx: 接受一个新的传入连接, 并可选地接收对方地址和初始数据, 常用于异步套接字服务器。
ConnectNamedPipe: 等待客户端连接到指定的命名管道实例, 用于实现服务端命名管道连接。
DeviceIoControl: 向设备驱动程序发送控制码, 用于执行自定义的设备操作, 如读写、获取状态等。
LockFileEx: 对文件的指定区域加锁, 支持共享锁或独占锁, 适用于进程间文件访问同步。
ReadDirectoryChangesW: 监视目录中的文件系统更改, 如文件创建、删除、重命名或修改等。
ReadFile: 从文件、管道、串口等读取数据, 支持 OVERLAPPED 结构实现异步读取。
TransactNamedPipe: 向命名管道写入请求并立即读取响应, 适用于单一事务的客户端-服务端通信。
WaitCommEvent: 等待串口通信事件发生, 如接收字符、状态位变化等, 用于串口异步事件通知。
WriteFile: 向文件、管道、串口等写入数据, 支持 OVERLAPPED 结构实现异步写入。
WSASendMsg: 向套接字发送带辅助控制信息的消息, 常用于实现高级协议特性如多播、IP 选项。
WSASendTo: 向指定地址的套接字发送数据, 适用于 UDP 等无连接协议的发送操作。
WSASend: 向已连接的套接字发送数据, 支持 OVERLAPPED 结构实现异步写入。
WSARecvFrom: 从套接字接收数据, 并获取发送方地址, 适用于 UDP 等无连接协议的接收操作。
LPFN_WSARECVMSG（WSARecvMsg）: 接收带有辅助控制信息的数据报, 用于处理如 IP_PKTINFO 等底层数据。
WSARecv: 从已连接的套接字接收数据, 支持 OVERLAPPED 结构实现异步读取。
*/

    /**
     * @brief 异步打开文件
     * @param dirfd 目录文件描述符, 它表示相对路径的基目录; `AT_FDCWD`, 则表示相对于当前工作目录
     * @param path 文件路径
     * @param flags 指定文件打开的方式, 比如 `O_RDONLY`
     * @param mode 文件权限模式, 仅在文件创建时有效 (一般写`0644`)
     * @return AioTask&& 
     */
    // [[nodiscard]] AioTask&& prepOpenat(
    //     int dirfd, 
    //     char const *path, 
    //     int flags,
    //     mode_t mode
    // ) && {
    //     // ::io_uring_prep_openat(_sqe, dirfd, path, flags, mode);
    //     return std::move(*this);
    // }

    /**
     * @brief 异步创建一个套接字
     * @param domain 指定 socket 的协议族 (AF_INET(ipv4)/AF_INET6(ipv6)/AF_UNIX/AF_LOCAL(本地))
     * @param type 套接字类型 SOCK_STREAM(tcp)/SOCK_DGRAM(udp)/SOCK_RAW(原始)
     * @param protocol 使用的协议, 通常为 0 (默认协议), 或者指定具体协议(如 IPPROTO_TCP、IPPROTO_UDP 等)
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] Task<::SOCKET> prepSocket(
        int domain, 
        int type, 
        int protocol,
        unsigned int flags
    ) && {
        // ::io_uring_prep_socket(_sqe, domain, type, protocol, flags);
        auto socket = ::WSASocket(domain, type, protocol, NULL, 0, flags);;
        if (socket == INVALID_SOCKET) [[unlikely]] {
            throw std::runtime_error{"socket ERROR: " + std::to_string(::WSAGetLastError())};
        }
        associateHandle(reinterpret_cast<::HANDLE>(socket));
        co_return socket;
    }

    /**
     * @brief 异步建立连接
     * @param fd 服务端套接字
     * @param addr [out] 客户端信息
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] Task<::SOCKET> prepAccept(
        ::SOCKET serSocket,
        struct ::sockaddr* addr, 
        // ::socklen_t *addrlen, // 内部决定
        int flags
    ) && {
        // ::io_uring_prep_accept(_sqe, fd, addr, addrlen, flags);
        /*
BOOL AcceptEx(
  [in]  SOCKET sListenSocket,         // 监听的socket <服务端套接字>
  [in]  SOCKET sAcceptSocket,         // 客户端套接字, 必须是通过socket函数创建的，未绑定的套接字
  [out] PVOID lpOutputBuffer,         // 用来接收首批数据及存储两个sockaddr结构的缓冲区
  [in]  DWORD dwReceiveDataLength,    // 在首次接受的数据中期望读取的字节数, 如果为0, 则表示不接收数据
  [in]  DWORD dwLocalAddressLength,   // 本地地址sockaddr结构的大小, 此值必须至少比正在使用的传输协议的最大地址长度多16个字节
  [in]  DWORD dwRemoteAddressLength,  // 远程地址sockaddr结构的大小, 此值必须至少比正在使用的传输协议的最大地址长度多16个字节
  [out] LPDWORD lpdwBytesReceived,    // 实际接收到的字节数
  [in, out] LPOVERLAPPED lpOverlapped // 指向OVERLAPPED结构的指针, 用于异步操作
);
        */
        // @todo 此处似乎仅支持 ipv4, ipv6 得想其他办法, 比如用宏
        // √8 iocp 和 AcceptEx 的设计... 居然要我们自己传入 new 的客户端套接字...
        auto cliSocket = 
            ::WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
            // ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (cliSocket == INVALID_SOCKET) [[unlikely]] {
            throw std::runtime_error{"socket ERROR: " + std::to_string(::WSAGetLastError())};
        }
        associateHandle(reinterpret_cast<::HANDLE>(cliSocket));
        
        char addrBuf[2 * (sizeof(::sockaddr) + 16)] {};
        ::DWORD bytes = 0;

        bool ok = ::AcceptEx(
            serSocket,
            cliSocket,
            addrBuf,
            0,
            sizeof(::sockaddr) + 16,
            sizeof(::sockaddr) + 16,
            &bytes,
            static_cast<::OVERLAPPED*>(this)
        );
        if (!ok && ::WSAGetLastError() != ERROR_IO_PENDING) [[unlikely]] {
            throw std::runtime_error{"AcceptEx ERROR: " + std::to_string(::GetLastError())};
        }
        return [](AioTask&& task, ::SOCKET _cliSocket) -> Task<::SOCKET> {
            co_await task;
            co_return _cliSocket;
        }(std::move(*this), cliSocket);
    }

    /**
     * @brief 异步的向服务端创建连接
     * @param fd 客户端套接字
     * @param addr [out] 服务端信息
     * @param addrlen 服务端信息长度指针
     * @return AioTask&& 
     */
    // [[nodiscard]] AioTask&& prepConnect(
    //     int fd, 
    //     const struct sockaddr *addr,
    //     socklen_t addrlen
    // ) && {
    //     // ::io_uring_prep_connect(_sqe, fd, addr, addrlen);
    //     return std::move(*this);
    // }

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
  LPDWORD      lpNumberOfBytesRead, // 实际读取的字节数（同步时非 NULL, 异步时设为 NULL）
  LPOVERLAPPED lpOverlapped         // OVERLAPPED 结构指针（异步时必填, 同步时为 NULL）
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
        associateHandle(fd);
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
        if (!ok && ::GetLastError() != ERROR_IO_PENDING) [[unlikely]] {
            throw std::runtime_error{"ReadFile ERROR: " + std::to_string(::GetLastError())};
        }
        return std::move(*this);
    }

    /**
     * @brief 异步读取文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param size 读取的长度
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRead(
        HANDLE fd,
        std::span<char> buf,
        unsigned int size,
        std::uint64_t offset
    ) && {
        // ::io_uring_prep_read(_sqe, fd, buf.data(), size, offset);
        associateHandle(fd);
        // 设置偏移量
        Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
        OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);
        bool ok = ::ReadFile(
            fd,
            buf.data(),
            static_cast<DWORD>(size),
            nullptr,
            static_cast<::OVERLAPPED*>(this)
        );
        if (!ok && ::GetLastError() != ERROR_IO_PENDING) [[unlikely]] {
            throw std::runtime_error{"ReadFile ERROR: " + std::to_string(::GetLastError())};
        }
        return std::move(*this);
    }

    /**
     * @brief 异步写入文件
     * @param fd 文件描述符
     * @param buf [in] 写入的数据
     * @param offset 文件偏移量
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepWrite(
        HANDLE fd, 
        std::span<char const> buf,
        std::uint64_t offset
    ) && {
        // ::io_uring_prep_write(_sqe, fd, buf.data(), static_cast<unsigned int>(buf.size()), offset);
        associateHandle(fd);
        Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
        OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);
        bool ok = ::WriteFile(
            fd,
            buf.data(),
            static_cast<DWORD>(buf.size()),
            nullptr,
            static_cast<::OVERLAPPED*>(this)
        );
        if (!ok && ::GetLastError() != ERROR_IO_PENDING) [[unlikely]] {
            throw std::runtime_error{"WriteFile ERROR: " + std::to_string(::GetLastError())};
        }
        return std::move(*this);
    }

    /**
     * @brief 异步读取网络套接字文件
     * @param fd 文件描述符
     * @param buf [out] 读取到的数据
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepRecv(
        ::SOCKET fd,
        std::span<char> buf,
        ::DWORD flags
    ) && {
        // ::io_uring_prep_recv(_sqe, fd, buf.data(), buf.size(), flags);
/*
int WSARecv(
  [in]  SOCKET                             s,                    // 目标套接字
  [in, out] LPWSABUF                       lpBuffers,            // 数据缓冲区数组
  [in]  DWORD                              dwBufferCount,        // 缓冲区数组中的元素数量
  [out] LPDWORD                            lpNumberOfBytesRecvd, // 实际接收的字节数 (IOCP时候为 nullptr)
  [in, out] LPDWORD                        lpFlags,              // 控制接收行为的标志
  [in, out] LPWSAOVERLAPPED                lpOverlapped,         // 指向OVERLAPPED结构的指针, 用于异步操作
  [in]  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine   // 完成例程的回调函数, 当I/O操作完成时被调用
);
*/
        ::WSABUF wsabuf {
            static_cast<::ULONG>(buf.size()),
            buf.data(),
        };
        int ok = ::WSARecv(
            fd,
            &wsabuf,
            1,
            nullptr,
            &flags,
            static_cast<::OVERLAPPED*>(this),
            nullptr
        );
        if (ok == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING) [[unlikely]] {
            throw std::runtime_error{"WSARecv ERROR: " + std::to_string(::WSAGetLastError())};
        }
        return std::move(*this);
    }

    /**
     * @brief 异步写入网络套接字文件
     * @param fd 文件描述符
     * @param buf [in] 写入的数据
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepSend(
        ::SOCKET fd, 
        std::span<char const> buf, 
        ::DWORD flags
    ) && {
        // ::io_uring_prep_send(_sqe, fd, buf.data(), buf.size(), flags);
/*
int WSASend(
  SOCKET                             s,                     // 套接字
  LPWSABUF                           lpBuffers,             // WSABUF
  DWORD                              dwBufferCount,         // lpBuffers数组中的元素数目
  LPDWORD                            lpNumberOfBytesSent,   // 返回实际发送的字节数
  DWORD                              dwFlags,               // 标志来控制发送操作的行为, 一般写 0
  LPWSAOVERLAPPED                    lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine    // 回调函数的指针
);
*/
        ::WSABUF wsabuf{
            static_cast<::ULONG>(buf.size()),
            const_cast<char*>(buf.data()),
        };
        int ok = ::WSASend(
            fd,
            &wsabuf,
            1,
            nullptr,
            flags,
            static_cast<::OVERLAPPED*>(this),
            nullptr
        );
        if (ok == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING) [[unlikely]] {
            throw std::runtime_error{"WSASend ERROR: " + std::to_string(::WSAGetLastError())};
        }
        return std::move(*this);
    }

    /**
     * @brief 异步关闭文件
     * @param fd 文件描述符
     * @return std::suspend_never 
     */
    [[nodiscard]] std::suspend_never prepClose(HANDLE fd) && {
        // ::io_uring_prep_close(_sqe, fd);
        auto&& runingHandleRef = _runingHandle.get();
        bool ok = ::CloseHandle(fd);
        if (!ok) [[unlikely]] {
            // 如果这里抛异常了, 那是不是无法关闭?!
            // 除非 fd 根本就不是由 win32 创建的?!
            // 原本的东西留着也没有用了...
            if (auto it = runingHandleRef.find(fd); it != runingHandleRef.end()) {
                runingHandleRef.erase(it);
            }
            throw std::runtime_error{"CloseHandle ERROR: " + std::to_string(::GetLastError())};
        }
        runingHandleRef.erase(fd);
        // @!!! 这里只能是同步的...
        return {};
    }

    /**
     * @brief 异步关闭套接字
     * @param socket 套接字
     * @return std::suspend_never 
     */
    [[nodiscard]] std::suspend_never prepClose(::SOCKET socket) && {
        // ::io_uring_prep_close(_sqe, fd);
        auto&& runingHandleRef = _runingHandle.get();
        int ok = ::closesocket(socket);
        if (ok == SOCKET_ERROR) [[unlikely]] {
            // 如果这里抛异常了, 那是不是无法关闭?!
            // 除非 fd 根本就不是由 win32 创建的?!
            // 原本的东西留着也没有用了...
            if (auto it = runingHandleRef.find(reinterpret_cast<::HANDLE>(socket));
                it != runingHandleRef.end()
            ) [[likely]] {
                runingHandleRef.erase(it);
            }
            throw std::runtime_error{"closesocket ERROR: " + std::to_string(::WSAGetLastError())};
        }
        runingHandleRef.erase(reinterpret_cast<::HANDLE>(socket));
        // @!!! 这里只能是同步的...
        return {};
    }

    /**
     * @brief 监测一个fd的pool事件
     * @param fd 需要监测的fd
     * @param pollMask 需要监测的poll事件 (如:`POLLIN`)
     * @return AioTask&& 
     */
    [[nodiscard]] AioTask&& prepPollAdd(
        int fd, 
        unsigned int pollMask
    ) && {
        // ::io_uring_prep_poll_add(_sqe, fd, pollMask);
        return std::move(*this);
    }

    /**
     * @brief 创建未链接的超时操作
     * @param ts 超时时间
     * @param flags 
     * @return AioTask&& 
     */
    [[nodiscard]] _AioTimeoutTask prepLinkTimeout(
        TimerLoop::TimerAwaiter&& timerTask
    ) && {
        // ::io_uring_prep_link_timeout(_sqe, ts, flags);
        return {std::move(*this), std::move(timerTask)};
    }

    [[nodiscard]] inline static auto linkTimeout(
        AioTask&& task, 
        _AioTimeoutTask&& timeoutTask
    ) {
        return [](AioTask&& _task,  _AioTimeoutTask&& _timeoutTask) 
        -> Task<HX::AwaiterReturnValue<decltype(whenAny(std::move(task), timeoutTask.co()))>> {
            _timeoutTask._self->_iocpHandle = _task._iocpHandle;
            co_return co_await whenAny(std::move(_task), _timeoutTask.co());
        }(std::move(task), std::move(timeoutTask));
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

    /**
     * @brief 是否还有任务在等待
     * @warning 如果您发现您卡在这里, 那么请检查是否有 fd 没有被 close!
     * @return true 还有任务
     * @return false 无任务
     */
    bool isRun() const {
        return _runingHandle.size();
    }

    void run(std::optional<std::chrono::system_clock::duration> timeout) {
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
        ::ULONG n = 0;
        decltype(toDwMilliseconds(*timeout)) dw = INFINITE;
        if (timeout) {
            dw = toDwMilliseconds(*timeout);
        }
        ::GetQueuedCompletionStatusEx(
            _iocpHandle,
            arr.data(),
            static_cast<::DWORD>(arr.size()),
            &n,
            dw,
            false
        );

        for (::ULONG i = 0; i < n; ++i) {
            auto ptr = arr[i];
            auto task = reinterpret_cast<AioTask*>(ptr.lpOverlapped);
            if (ptr.lpCompletionKey == static_cast<::ULONG_PTR>(AioTask::State::Cancel)) {
                print::println("已取消");
                continue;
            }
            task->_res = ptr.dwNumberOfBytesTransferred;
            _tasks.push_back(task->_previous);
        }

        for (const auto& t : _tasks) {
            t.resume();
        }

        _tasks.clear();
    }

    ~Iocp() noexcept {
        if (_iocpHandle) {
            ::CloseHandle(_iocpHandle);
        }
    }

private:
    HANDLE _iocpHandle;
    std::unordered_set<::HANDLE> _runingHandle;
    std::vector<std::coroutine_handle<>> _tasks;
};

struct Loop {
    void start() {
        auto tasks = test5();
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
        
        DWORD mode = 0;
        if (!GetConsoleMode(hStdin, &mode)) {
            std::cerr << "GetConsoleMode failed\n";
            CloseHandle(hStdin);
            co_return;
        }
        
        while (true) {
            std::string buf;
            buf.resize(128);
            std::cerr << "cin >> ";
            try {
                auto res = co_await AioTask::linkTimeout(
                    _iocp.makeAioTask().prepRead(
                        hStdin,
                        buf,
                        0
                    ),
                    _iocp.makeAioTask().prepLinkTimeout(
                        makeTimer().sleepFor(5s)
                    )
                );
                print::println("res: ", res.index());
                if (res.index() == 1) {
                    print::print("超时了~");
                    co_await _iocp.makeAioTask().prepClose(hStdin);
                    co_return;
                }
            } catch (std::exception& e) {
                std::cerr << "fxxk throw: " << e.what();
                co_return;
            }
            buf[buf.find('\n')] = '\0'; // 因为回车才有输入, 所以必然有回车
            print::println("echo: ", buf);
        }
        co_return;
    }

    // 普通文件读写
    Task<> test2() {
        HANDLE file = ::CreateFileA(
            "test.txt",                        // 文件名
            GENERIC_READ | GENERIC_WRITE,     // 读写权限
            FILE_SHARE_READ | FILE_SHARE_WRITE, // 共享模式
            nullptr,                          // 默认安全属性
            OPEN_ALWAYS,                      // 如果文件不存在则创建
            FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, // 必须带 FILE_FLAG_OVERLAPPED
            nullptr                           // 不拷贝句柄
        );
        std::string buf;
        buf.resize(1024);
        co_await _iocp.makeAioTask().prepRead(file, buf, 0);
        print::println("cout << ", buf);
        // @todo 目前会报错
        // 原因是对同一个 fd 绑定了多次 iocp
        // 想必之前的输入流也是这个原因...
        // 解决方案:
        // 1) 每次协程co_await结束, 解绑 iocp: CreateIoCompletionPort((HANDLE)cliSocket, NULL, 0, 0);
        // 但是实际上这个是错误的, ... 并没有解绑; 并且绑定 NULL, 本质上就是错误的...
        // https://devblogs.microsoft.com/oldnewthing/20130823-00/?p=3423
        // 只能通过 CloseHandle || closesocket 来让内核解除关联...
        // 2) CreateIoCompletionPort 时候, 判断失败是否为 ERROR_INVALID_PARAMETER (87)
        // 然后忽略错误, 表示已经绑定... (但是会有一次内核态的切换...)
        // 3) 采用 std::unordered_set 来记录哪些已经绑定了; 最后只需要在 CloseHandle / closesocket 时候erase即可
        // 但是常数不定..., 但是应该是最优的方案了吧?!
        co_await _iocp.makeAioTask().prepWrite(file, "fuck msvc!\nfuck windous", 0);
        co_await _iocp.makeAioTask().prepClose(file);
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

    // 测试分离协程
    Task<> test4() {
        using namespace std::chrono;
        for (int i = 0; ; ++i) {
            print::println("make id: ", i, " {");
            test4_sub(i).detach();
            print::println("} // make id: ", i);
            co_await makeTimer().sleepFor(2s);
        }
        co_return;
    }

    RootTask<> test4_sub(int id) {
        using namespace std::chrono;
        struct __raii_test_ {
            ~__raii_test_() noexcept {
                print::println("~test4_sub(): , ", _id, "\n");
            }

            int _id;
        } _{id};
        for (int i = 0; i < 5; ++i) {
            print::println("The ", id, " sleep (", i , ")");
            co_await makeTimer().sleepFor(1s);
        }
        co_return;
    }

    // 测试网络请求
    Task<> test5() {
        // 1. 等待连接
        auto serSocket 
            = co_await _iocp.makeAioTask()
                            .prepSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
        try {

            sockaddr_in addr;
            addr.sin_family = AF_INET;   // 地址为IPv4协议
            addr.sin_port = htons(28205); // 端口为 28205
            addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0"); // 具体绑定本机的地址

            if (::bind(serSocket, reinterpret_cast<::sockaddr*>(&addr),
                       sizeof(addr)) == SOCKET_ERROR) [[unlikely]] {
                throw std::runtime_error{"bind error!"};
            }

            if (::listen(serSocket, 64) == SOCKET_ERROR) [[unlikely]] {
                throw std::runtime_error{"listen error!"};
            }

            for(;;) {
                print::println("等待连接...");
                auto cliFd 
                    = co_await _iocp.makeAioTask()
                                    .prepAccept(serSocket, nullptr, 0);
                // 2. func: 仅处理连接, 需要分离协程
                /*
                期望调度:
                1)
                    根协程 -> prepAccept 协程 -> 得到 cliSocket -> 创建 cli协程

                2)
                    根协程 -> prepAccept 协程
                    根协程 -> cli协程
                */
                print::println("新连接!");
                comm(cliFd).detach();
            }
        } catch (std::exception& err) {
            print::println("err: ", err.what());
        }
        co_await _iocp.makeAioTask().prepClose(serSocket);
        co_return;
    }

    RootTask<> comm(::SOCKET cliFd) {
        using namespace std::chrono;
        std::string buf;
        for(;;) {
            buf.clear();
            buf.resize(1024);
            print::println("等待读取...");
            try {
                // 读
                auto res = co_await AioTask::linkTimeout(
                    _iocp.makeAioTask().prepRecv(cliFd, buf, 0),
                    _iocp.makeAioTask().prepLinkTimeout(
                        makeTimer().sleepFor(3s)
                    )
                );
                if (res.index() == 1) [[unlikely]] {
                    break;
                }

                buf = "Hello! " + buf;

                print::println(buf);

                // 写
                auto sendSize = co_await _iocp.makeAioTask()
                                                        .prepSend(cliFd, buf, 0);

                print::println("已发送... ", sendSize);
            } catch (std::exception& ec) {
                using namespace std::string_literals;
                print::println(ec.what());
            }
        }
        print::println("断线, ", cliFd);
        co_await _iocp.makeAioTask().prepClose(cliFd);
        print::println("已断线: ", cliFd);
        struct __raii__ {
            ~__raii__() noexcept {
                print::println("RAII Clone: comm_task");
            }
        } _;
        co_return;
    }

    Iocp _iocp;
    TimerLoop _timerLoop;
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
    try {
        Loop loop;
        loop.start();
    } catch (std::exception& ec) {
        print::println("ERR: ", ec.what());
    }
    // constexpr bool _ = AwaitableLike<Task<>>; 
    HX::print::println("END: main");
    return 0;
}