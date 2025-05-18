#include <Hxprint/print.h>
#include <coroutine>

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

template <typename T>
struct NoVoidRes {
    using type = T;
};

template <>
struct NoVoidRes<void> {
    using type = NoVoidRes;
};

// 包裹一层, 防止 std::optional<void> 编译报错
template <typename T>
using NoVoidResType = NoVoidRes<T>::type;

template <typename T>
struct Promise;

template <typename T = void>
struct Task {
    using promise_type = Promise<T>;

    std::optional<NoVoidResType<T>> operator()() noexcept {
        if (_coroutine.done())
            return {};
        _coroutine.resume();
        if constexpr (!std::is_void_v<T>) {
            return _coroutine.promise()._data;
        }
        return {};
    }

    template <bool Sleep>
    struct Awaiter {
        constexpr bool await_ready() const noexcept { return !Sleep; }
        constexpr auto await_suspend(std::coroutine_handle<> nowCoroutine) noexcept {
            _coroutine.promise()._mae = nowCoroutine;
            return _coroutine;
        }
        constexpr NoVoidResType<T> await_resume() const noexcept { 
            return *_coroutine.promise()._data;;
        }
        std::coroutine_handle<promise_type> _coroutine;
    };
    
    Awaiter<true> operator co_await() {
        return {_coroutine};
    }

    ~Task() noexcept {
        if (_coroutine)
            _coroutine.destroy();
    }

    std::coroutine_handle<promise_type> _coroutine{nullptr};
};

struct MaeAwaiter {
    constexpr bool await_ready() const noexcept { return false; }
    constexpr std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept {
        if (_mae)
            return _mae;
        return std::noop_coroutine();
    }
    constexpr void await_resume() const noexcept {}
    std::coroutine_handle<> _mae{nullptr};
};

template <typename T>
struct Promise {
    SleepAwaiter<true> initial_suspend() noexcept { return {}; }
    Task<T> get_return_object() noexcept {
        return {std::coroutine_handle<Promise<T>>::from_promise(*this)};
    }
    MaeAwaiter final_suspend() noexcept { return {_mae}; }
    void unhandled_exception() { throw; }
    void return_value(T&& t) { _data = std::forward<T>(t); }
    SleepAwaiter<true> yield_value(T&& t) { _data = std::forward<T>(t); return {}; }
    
    std::optional<NoVoidResType<T>> _data{};
    std::coroutine_handle<> _mae{};
    ~Promise() noexcept { HX::print::println("~Promise"); }
};

template <>
struct Promise<void> {
    SleepAwaiter<true> initial_suspend() noexcept { return {}; }
    Task<void> get_return_object() noexcept {
        return {std::coroutine_handle<Promise<void>>::from_promise(*this)};
    }
    MaeAwaiter final_suspend() noexcept { return {_mae}; }
    void unhandled_exception() { throw; }
    void return_void() noexcept { }
    // SleepAwaiter<true> yield_value() { return {}; } // 这个不是协程函数

    std::coroutine_handle<> _mae{};
    ~Promise() noexcept { HX::print::println("~Promise<void>"); }
};

Task<int> test() {
    HX::print::println("test");
    co_yield 666;
    HX::print::println("test mo i kai~");
    co_return 2233;
}

Task<> testVoid() {
    HX::print::println("test return void {");
    auto res = co_await []() -> Task<std::string> {
        HX::print::println("in the co_await");
        co_return std::string{"123"};
    }();
    HX::print::println("} // test return void: ", res);
    co_return;
}

int main() {
    {
        auto task = test();
        auto res = task();
        HX::print::println("R 1: res = ", res);
        res = task();
        HX::print::println("R 2: res = ", res);
        res = task();
        HX::print::println("End: res = ", res);
    }
    {
        auto task = testVoid();
        auto res = task();
        HX::print::println("End: res = ", res);
    }
    return 0;
}