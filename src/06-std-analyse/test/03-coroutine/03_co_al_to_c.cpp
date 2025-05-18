/*************************************************************************************
 * NOTE: The coroutine transformation you've enabled is a hand coded
 *transformation! * Most of it is _not_ present in the AST. What you see is an
 *approximation.   *
 *************************************************************************************/

// #define __co_al_to_cpp__
#ifdef __co_al_to_cpp__

#include <coroutine>
#include <iostream>
#include <thread>

namespace Coroutine {
struct Task {
    struct promise_type {
        inline promise_type() {
            std::operator<<(std::cout, static_cast<const char*>(
                                           "1.create promie object\n"));
        }

        inline Coroutine::Task get_return_object() {
            std::operator<<(
                std::cout,
                static_cast<const char*>(
                    "2.create coroutine return object, and the coroutine is created now\n"));
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        inline std::suspend_never initial_suspend() {
            std::operator<<(
                std::cout,
                static_cast<const char*>(
                    "3.do you want to susupend the current coroutine?\n"));
            std::operator<<(
                std::cout,
                static_cast<const char*>(
                    "4.don't suspend because return std::suspend_never, so continue to execute coroutine body\n"));
            return {};
        }

        inline std::suspend_never final_suspend() noexcept {
            std::operator<<(
                std::cout,
                static_cast<const char*>(
                    "13.coroutine body finished, do you want to susupend the current coroutine?\n"));
            std::operator<<(
                std::cout,
                static_cast<const char*>(
                    "14.don't suspend because return std::suspend_never, and the continue will be automatically destroyed, bye\n"));
            return {};
        }

        inline void return_void() {
            std::operator<<(
                std::cout,
                static_cast<const char*>(
                    "12.coroutine don't return value, so return_void is called\n"));
        }

        inline void unhandled_exception() {
        }

        inline ~promise_type() noexcept {
            std::operator<<(std::cout,
                            static_cast<const char*>("15. ~promise_type()\n"));
        }
    };

    inline ~Task() noexcept {
        std::operator<<(std::cout, static_cast<const char*>("9.9 ~Task()\n"));
    }

    std::coroutine_handle<promise_type> handle_;
};

struct Awaiter {
    inline bool await_ready() {
        std::operator<<(std::cout,
                        static_cast<const char*>(
                            "6.do you want to suspend current coroutine?\n"));
        std::operator<<(
            std::cout,
            static_cast<const char*>(
                "7.yes, suspend becase awaiter.await_ready() return false\n"));
        return false;
    }

    inline void
    await_suspend(std::coroutine_handle<Task::promise_type> handle) {
        std::operator<<(std::cout, static_cast<const char*>(
                                       "8.execute awaiter.await_suspend()\n"));

        class __lambda_68_21 {
        public:
            inline /*constexpr */ void operator()() {
                static_cast<
                    const std::coroutine_handle<Coroutine::Task::promise_type>>(
                    handle)
                    .operator()();
            }

        private:
            std::coroutine_handle<Coroutine::Task::promise_type> handle;

        public:
            // inline /*constexpr */ __lambda_68_21 & operator=(const
            // __lambda_68_21 &) /* noexcept */ = delete; inline /*constexpr */
            // __lambda_68_21(__lambda_68_21 &&) noexcept = default;
            __lambda_68_21(
                const std::coroutine_handle<Coroutine::Task::promise_type>&
                    _handle) : handle{_handle} {
            }
        };

        __lambda_68_21 __temporary68_52 = __lambda_68_21{static_cast<
            const std::coroutine_handle<Coroutine::Task::promise_type>>(
            handle)};
        std::thread __temporary68_53 =
            std::thread(std::thread(__temporary68_52));
        __temporary68_53.detach();
        __temporary68_53.~thread();
        /* __temporary68_52 // lifetime ends here */
        std::operator<<(
            std::cout,
            static_cast<const char*>(
                "9.a new thread lauched, and will return back to caller\n"));
    }

    inline void await_resume() {
    }

    inline ~Awaiter() noexcept {
        std::operator<<(std::cout, static_cast<const char*>("~Awaiter()\n"));
    }
};

struct __testFrame {
    void (*resume_fn)(__testFrame*);
    void (*destroy_fn)(__testFrame*);
    std::__coroutine_traits_impl<Task>::promise_type __promise;
    int __suspend_index;
    bool __initial_await_suspend_called;
    std::suspend_never __suspend_80_6;
    Awaiter __suspend_84_14;
    std::suspend_never __suspend_80_6_1;
};

Task test() {
    /* Allocate the frame including the promise */
    /* Note: The actual parameter new is __builtin_coro_size */
    __testFrame* __f =
        reinterpret_cast<__testFrame*>(operator new(sizeof(__testFrame)));
    __f->__suspend_index = 0;
    __f->__initial_await_suspend_called = false;

    /* Construct the promise. */
    new (&__f->__promise) std::__coroutine_traits_impl<Task>::promise_type{};

    /* Forward declare the resume and destroy function. */
    void __testResume(__testFrame * __f);
    void __testDestroy(__testFrame * __f);

    /* Assign the resume and destroy function pointers. */
    __f->resume_fn = &__testResume;
    __f->destroy_fn = &__testDestroy;

    /* Call the made up function with the coroutine body for initial suspend.
     This function will be called subsequently by coroutine_handle<>::resume()
     which calls __builtin_coro_resume(__handle_) */
    __testResume(__f);

    return __f->__promise.get_return_object();
}

/* This function invoked by coroutine_handle<>::resume() */
void __testResume(__testFrame* __f) {
    try {
        /* Create a switch to get to the correct resume point */
        switch (__f->__suspend_index) {
        case 0: break;
        case 1: goto __resume_test_1;
        case 2: goto __resume_test_2;
        case 3: goto __resume_test_3;
        }
        __temporary80_6 = __f->__promise.initial_suspend();
        __temporary80_6 =
            std::coroutine_handle<Task::promise_type>::from_address(
                static_cast<void*>(__f));

        /* co_await insights.cpp:80 */
        __f->__suspend_80_6 = __f->__promise.initial_suspend();
        if (!__f->__suspend_80_6.await_ready();) {
            __f->__suspend_80_6.await_suspend(
                __temporary80_6.operator std::coroutine_handle<void>());
            ;
            __f->__suspend_index = 1;
            __f->__initial_await_suspend_called = true;
            return;
            /* __temporary80_6 // lifetime ends here */
            /* __temporary80_6 // lifetime ends here */
        }

    __resume_test_1:
        __f->__suspend_80_6.await_resume();
        /* __temporary80_6 // lifetime ends here */
        /* __temporary80_6 // lifetime ends here */
        std::operator<<(
            std::operator<<(
                std::operator<<(
                    std::cout,
                    "5.begin to execute coroutine body, the thread id="),
                std::this_thread::get_id()),
            "\n");
        __temporary84_22 = Awaiter{};

        /* co_await insights.cpp:84 */
        __f->__suspend_84_14 = Awaiter{};
        if (!__f->__suspend_84_14.await_ready();) {
            __f->__suspend_84_14.await_suspend(
                std::coroutine_handle<Task::promise_type>::from_address(
                    static_cast<void*>(__f)));
            __f->__suspend_index = 2;
            return;
            __temporary84_22.~Awaiter();
        }

    __resume_test_2:
        __f->__suspend_84_14.await_resume();
        __temporary84_22.~Awaiter();
        std::operator<<(
            std::operator<<(
                std::operator<<(
                    std::cout,
                    "11.coroutine resumed, continue execcute coroutine body now, the thread id="),
                std::this_thread::get_id()),
            "\n");
        /* co_return insights.cpp:80 */
        __f->__promise.return_void() /* implicit */;
        goto __final_suspend;
    } catch (...) {
        if (!__f->__initial_await_suspend_called) {
            throw;
        }

        __f->__promise.unhandled_exception();
        ;
    }

__final_suspend:
    __temporary80_6 = __f->__promise.final_suspend();
    __temporary80_6 = std::coroutine_handle<Task::promise_type>::from_address(
        static_cast<void*>(__f));

    /* co_await insights.cpp:80 */
    __f->__suspend_80_6_1 = __f->__promise.final_suspend();
    if (!__f->__suspend_80_6_1.await_ready();) {
        __f->__suspend_80_6_1.await_suspend(
            __temporary80_6.operator std::coroutine_handle<void>());
        ;
        __f->__suspend_index = 3;
        return;
        /* __temporary80_6 // lifetime ends here */
        /* __temporary80_6 // lifetime ends here */
    }

__resume_test_3:
    __f->destroy_fn(__f);
    /* __temporary80_6 // lifetime ends here */
    /* __temporary80_6 // lifetime ends here */
}

/* This function invoked by coroutine_handle<>::destroy() */
void __testDestroy(__testFrame* __f) {
    /* destroy all variables with dtors */
    __f->~__testFrame();
    /* Deallocating the coroutine frame */
    /* Note: The actual argument to delete is __builtin_coro_frame with the
     * promise as parameter */
    operator delete(static_cast<void*>(__f), sizeof(__testFrame));
}

} // namespace Coroutine

int main() {
    Coroutine::test();
    ;
    std::operator<<(
        std::cout, static_cast<const char*>(
                       "10.come back to caller becuase of co_await awaiter\n"));
    const int __temporary96_54 = static_cast<const int>(1);
    const std::chrono::duration<long, std::ratio<1, 1>> __temporary96_55 =
        static_cast<const std::chrono::duration<long, std::ratio<1, 1>>>(
            std::chrono::duration<long, std::ratio<1, 1>>(__temporary96_54));
    std::this_thread::sleep_for<long, std::ratio<1, 1>>(__temporary96_55);
    /* __temporary96_55 // lifetime ends here */
    /* __temporary96_54 // lifetime ends here */
    return 0;
}

#endif // !__co_al_to_cpp__