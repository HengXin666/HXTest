#include <HXprint/print.h>
#include <coroutine>

namespace hx {

/**
 * @brief 等价于标准库的 `std::suspend_always`/`suspend_never`
 */
template <bool Sleep>
struct SleepAwaiter {
    /**
     * @brief await SleepAwaiter{} 时候, 会首先调用 `await_ready()`, 并根据返回值见机行事
     * @return true  将调用 `await_resume()` 以恢复协程执行
     * @return false 将调用 `await_suspend()` 以暂停当前协程, 然后转移执行权
     */
    constexpr bool await_ready() const noexcept { return !Sleep; }
    constexpr auto await_suspend(std::coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

}

struct RepeatAwaiter {
    bool await_ready() const noexcept { 
        return false;
    }

    /**
     * @brief 它的返回值决定了协程在挂起后是否以及如何恢复。这个返回值类型是灵活的!
     * @param coroutine 
     * @return 可以返回指向同一个协程、另一个协程、或 std::noop_coroutine() 的句柄。
     * 
     * std::noop_coroutine():
     * 返回这个特殊的协程句柄表示没有实际的工作需要做。
     * 这意味着当前协程被挂起后, 不需要恢复任何协程的执行。这通常用于表示协程已经完成了其目的, 没有后续操作。
     * 
     * std::coroutine_handle<>::from_address(nullptr) 或等效值:
     * 指示协程已完成, 其协程帧应该被销毁。这种情况下, 控制权返回给调度器或协程的调用者。
     * 
     * coroutine (传入的协程句柄):
     * 返回传入的协程句柄表示恢复执行这个协程。这通常用于协程之间的切换或控制流的转移。
     */
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        // (每次co_yield)停止执行
        // return std::noop_coroutine();

        // 如果 coroutine 可以执行就继续执行
        // if (coroutine.done())
        //     return std::noop_coroutine();
        // else
            return coroutine; // 继续执行本协程
    }

    void await_resume() const noexcept {}
};

struct Promise {
    // 初次构造时候是否执行
    auto initial_suspend() {
        return hx::SleepAwaiter<true>{}; // 返回协程控制权
    }

    // 最后结束时候是否暂停协程
    auto final_suspend() noexcept {
        return std::suspend_always();
    }

    void unhandled_exception() {
        throw;
    }

    auto yield_value(int ret) {
        _resValue = ret;
        return RepeatAwaiter(); // 必须满足所谓的规范
    }

    void return_void() {
        _resValue = 0;
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    int _resValue;
};

/// @brief 定义协程任务句柄
struct Task {
    using promise_type = Promise; // 协程任务 必需要包含 promise_type

    Task(std::coroutine_handle<promise_type> coroutine)
        : _coroutine(coroutine) 
    {}

    struct Awaiter {
        bool await_ready() const noexcept { return false; }
        auto await_suspend(std::coroutine_handle<promise_type> coroutine) noexcept {
            if (coroutine == _coroutine)
                HX::print::println("is ==");
            else
                HX::print::println("is !=");
            return _coroutine;
        }
        void await_resume() noexcept {}

        std::coroutine_handle<promise_type> _coroutine;
    };

    Awaiter operator co_await() {
        // 此处传入的是 Task 的协程句柄 吗?
        return {_coroutine};
    }

    std::coroutine_handle<promise_type> _coroutine; // 协程句柄
};

Task hello() {
    HX::print::println("hello 42");
    co_await []() -> Task {
        HX::print::println("await {");
        co_yield 2233;
        HX::print::println("} // 2233");
        co_return;
    }();
    co_yield 42;
    HX::print::println("hello 12");
    co_yield 12;
    HX::print::println("hello 6");
    co_yield 6;
    HX::print::println("hello end");
    co_return;
}

int main() {
    HX::print::println("main run hello");
    Task t = hello();
    HX::print::println("main run hello end!");
    // 实际上上面什么也没有做(并没有调用hello) 
    // 因为 initial_suspend 是 return std::suspend_always(); // 返回协程控制权

    while (!t._coroutine.done()) {
        t._coroutine.resume();
        HX::print::println("main get return val: ", t._coroutine.promise()._resValue);
    }
    return 0;
}