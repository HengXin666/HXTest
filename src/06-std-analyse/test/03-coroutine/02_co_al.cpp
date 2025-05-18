#include <coroutine>
#include <iostream>
#include <thread>

/**
 * @brief from https://zhuanlan.zhihu.com/p/497224333
 */

namespace Coroutine {

template <bool Sleep, typename T = void>
struct SleepAwaiter {
    /**
     * @brief await SleepAwaiter{} 时候, 会首先调用 `await_ready()`, 并根据返回值见机行事
     * @return true  将调用 `await_resume()` 以恢复协程执行
     * @return false 将调用 `await_suspend()` 以暂停当前协程, 然后转移执行权
     */
    constexpr bool await_ready() const noexcept { return !Sleep; }
    constexpr auto await_suspend(std::coroutine_handle<>) const noexcept {
        std::cout << "??. await_suspend!\n";
    }
    constexpr T await_resume() const noexcept {
        std::cout << "???. await_resume!\n";
    }
};


struct Task {
    struct promise_type {
        explicit promise_type() {
            // 创建promie对象
            std::cout << "1.create promie object\n";
        }
        Task get_return_object() {
            std::cout // 创建协程返回对象，现在创建协程
                << "2.create coroutine return object, and the coroutine is created now\n";
            return {
                std::coroutine_handle<Task::promise_type>::from_promise(*this)
            };
        }
        SleepAwaiter<false> initial_suspend() {
            // 你想监视当前的协程吗?
            std::cout << "3.do you want to susupend the current coroutine?\n";
            std::cout // 不要挂起, 因为返回std:：suspend_never, 所以继续执行协程体
                << "4.don't suspend because return std::suspend_never, so continue to execute coroutine body\n";
            return {};
        }
        SleepAwaiter<true> final_suspend() noexcept {
            std::cout // 协程主体已执行完毕, 你是否希望挂起当前协程?
                << "13.coroutine body finished, do you want to susupend the current coroutine?\n";
            std::cout // 不要挂起, 因为返回std:：suspended_never, continue 将被自动销毁, 再见
                << "14.don't suspend because return std::suspend_never, and the continue will be automatically destroyed, bye\n";
            return {};
        }
        void return_void() {
            std::cout // 协程不返回值, 因此调用return_void
                << "12.coroutine don't return value, so return_void is called\n";
        }
        void unhandled_exception() {

        }

        ~promise_type() {
            // 如果修改 final_suspend 返回 suspend_always, 那么将不会输出这里
            std::cout << "15. ~promise_type()\n";
        }
    };

    ~Task() {
        std::cout << "9.9 ~Task()\n";
    }

    std::coroutine_handle<Task::promise_type> handle_;
};

struct Awaiter {
    Awaiter() {
        std::cout << "Awaiter() make\n";
    }

    bool await_ready() {
        // 您想暂停当前的协程吗
        std::cout << "6.do you want to suspend current coroutine?\n";
        std::cout // 是的, suspend变为awaiter.await_ready()返回false
            << "7.yes, suspend becase awaiter.await_ready() return false\n";
        return false;
    }
    auto await_suspend(std::coroutine_handle<Task::promise_type> handle) {
        // 执行awaiter.await_suspend()
        std::cout << "8.execute awaiter.await_suspend()\n";
        // std::thread([handle]() mutable { handle(); }).detach();
        // 一个新线程已启动, 并将返回给调用者
        std::cout << "9.a new thread lauched, and will return back to caller\n";

        return handle;
    }
    int await_resume() {
        std::cout << "xxxx\n";
        return 1;
    }

    ~Awaiter() {
        std::cout << "~Awaiter()\n";
    }
};

Task test() {
    // 开始执行协程体, 线程id=11732
    std::cout << "5.begin to execute coroutine body, the thread id="
              << std::this_thread::get_id() << "\n"; // #1
    auto res = co_await Awaiter{};
    // 协程恢复, 现在继续执行协程体, 线程id=11280
    std::cout << "11.coroutine resumed, continue execcute coroutine body now, "
                 "the thread id="
              << std::this_thread::get_id() << "\n"; // #3
    (void)res;
}
} // namespace Coroutine

int main() {
    char a[] = "123";
    std::cout << (&a) << "\n";

    auto res = Coroutine::test();

    std::cout << "?. \n";
    // res.handle_.resume();
    // 由于执行了 co_await awaiter, 当前协程挂起, 并将控制权返回给调用方
    std::cout << "10.come back to caller becuase of co_await awaiter\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}