#include <HXprint/print.h>
#include <coroutine/awaiter/WhenAny.hpp>

#include <list>

/**
 * @brief 尝试编写一个协程(线程)池 [[不如人意]]
 */

namespace HX {

struct CoPool {
    using CoPoolQueue = std::list<std::coroutine_handle<>>;
    CoPoolQueue _q; // 调度队列
    
    enum class CoPoolTag {
        Push,
        PushAndSwitch,
        Switch,
    };

    template <CoPoolTag IsResume>
    struct CoPoolAwaiter {
        constexpr bool await_ready() const noexcept { return false; }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            if constexpr (IsResume == CoPoolTag::Push) {
                _q.push_back(coroutine);
                return coroutine;
            } else if constexpr (IsResume == CoPoolTag::PushAndSwitch) {
                _q.push_back(coroutine);
                auto res = _q.front();
                _q.pop_front();
                return res;
            } else { // IsResume == CoPoolTag::Switch
                if (_q.empty())
                    return std::noop_coroutine();
                auto res = _q.front();
                _q.pop_front();
                return res;
            }
        }
        constexpr void await_resume() const noexcept {}

        CoPoolQueue& _q;
    };

    void run() {
        while (_q.size()) {
            _q.front().resume();
            _q.pop_front();
        }
    }

/*
期望是:
make: 0 {
        [1] ---> by: 0
} // make cnt: 0
make: 1 {
        [1] ---> by: 1
        [2] ---> by: 0
} // make cnt: 1
*/

    Task<> co_main() {
        std::size_t cnt = 0;
        for (int i = 1; i <= 5; ++i) {
            co_await CoPoolAwaiter<CoPoolTag::Push>{_q};
            print::println("make: ", cnt, " {");
            co_await [](std::size_t cnt, CoPoolQueue& _q) -> Task<
                void, Promise<void, StopAwaiter<true>, StopAwaiter<false>>
            > {
                struct A {
                    ~A() noexcept { print::println("~A"); } // 这个会被泄露 ...
                } _;
                print::println("\t[1] ---> by: ", cnt);
                co_await CoPoolAwaiter<CoPoolTag::PushAndSwitch>{_q};
                print::println("\t[2] ---> by: ", cnt);
                co_await CoPoolAwaiter<CoPoolTag::PushAndSwitch>{_q};
                print::println("\t[3] ---> by: ", cnt);
                co_return co_await CoPoolAwaiter<CoPoolTag::Switch>{_q};
            }(cnt, _q);
            print::println("} // make cnt: ", cnt);
            ++cnt;
        }
        co_return 
        // run()
        ;
    }

    Task<Task<std::size_t>> makeWork() {
        std::size_t cnt = 0;
        while (true) {
            co_await CoPoolAwaiter<CoPoolTag::Push>{_q};
            print::println("make: ", cnt, " {");
            co_yield [](std::size_t cnt, CoPoolQueue& _q) -> Task<std::size_t> {
                print::println("[1] cnt: ", cnt);
                co_await CoPoolAwaiter<CoPoolTag::PushAndSwitch>{_q};
                print::println("[2] ---> cnt: ", cnt);
                co_return cnt;
            }(cnt, _q);
            ++cnt;
        }
        co_return {};
    }
    
    Task<> co_main02() {
        auto make = makeWork();
        for (int i = 0; i < 5; ++i) {
            print::println("} // return cnt: ", co_await co_await make);
        }
        co_return;
    }
};

} // namespace HX

int main() {
    return static_cast<std::coroutine_handle<>>(HX::CoPool{}.co_main()).resume(), 0;
}