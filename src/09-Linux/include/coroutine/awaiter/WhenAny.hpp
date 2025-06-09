#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-09 23:09:27
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HX_WHEN_ANY_H_
#define _HX_WHEN_ANY_H_

#include <utility>
#include <array>
#include <variant>

#include <tools/Uninitialized.hpp>
#include <coroutine/concepts/Awaiter.hpp>
#include <coroutine/task/Task.hpp>

namespace HX {

namespace internal {

/**
 * @brief WhenAny 控制块
 */
struct WhenAnyCtlBlock {
    std::coroutine_handle<> previous;
};

struct WhenAnyAwaiter {
    constexpr bool await_ready() const noexcept { return false; }
    constexpr auto await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        ctlBlock.previous = coroutine;
        return _coroutine;
    }
    constexpr void await_resume() const noexcept {}

    std::coroutine_handle<> _coroutine;
    WhenAnyCtlBlock& ctlBlock;
};

struct WhenAnyTask {
    using promise_type = Promise<void>;

    WhenAnyTask(std::coroutine_handle<promise_type> h)
        : _handle(h)
    {}

    auto operator co_await() noexcept {
        return WhenAnyAwaiter{_handle, *ctlBlockPtr};
    }

    ~WhenAnyTask() { if (_handle) _handle.destroy(); }

    std::coroutine_handle<promise_type> _handle;
    WhenAnyCtlBlock* ctlBlockPtr{};
};

template <std::size_t Idx, Awaiter T, typename Arr>
Task<std::coroutine_handle<>> start(
    T&& t, Arr& resArr, std::size_t& resIdx, WhenAnyCtlBlock& ctlBlock
) {
    if (ctlBlock.previous == std::noop_coroutine()) {
        co_return std::noop_coroutine();
    }

    if constexpr (!std::is_void_v<decltype(t.operator co_await().await_resume())>) {
        resArr[Idx] = co_await t;
    } else {
        co_await t;
    }
    resIdx = Idx;

    // !!! @todo !!!
    auto res = ctlBlock.previous;
    ctlBlock.previous = std::noop_coroutine();
    co_return res;
}

template <
    std::size_t... Idx, 
    Awaiter... Ts, 
    typename ResType = std::variant<
        typename NonVoidHelper<decltype(
                std::declval<Ts>().operator co_await().await_resume()
            )>::Type...>
>
Task<ResType> whenAny(std::index_sequence<Idx...>, Ts&&... ts) {
    // 1. 计算返回值
    // using ResType = std::variant<NonVoidHelper<decltype(std::declval<Ts>().await_resume())>...>;

    // 2. 存储所有的返回值
    std::array<ResType, sizeof...(Ts)> resArr;
    std::size_t resIdx = static_cast<std::size_t>(-1);

    // 3. 启动所有协程
    WhenAnyCtlBlock block;
    WhenAnyTask wyTask = [&]() -> WhenAnyTask {
        (
            (co_await start<Idx>(std::forward<Ts>(ts), resArr, resIdx, block)).resume()
        , ...);
        co_return;
    }();
    wyTask.ctlBlockPtr = &block;

    // 4. 等待其中一个 (挂起)
    co_await wyTask;

    // 5. 通过返回值确定返回谁
    co_return resArr[resIdx];
}

} // namespace internal

template <Awaiter... Ts>
auto whenAny(Ts&&... ts) {
    return internal::whenAny(
        std::make_index_sequence<sizeof...(ts)>(), 
        std::forward<Ts>(ts)...
    );
}

} // namespace HX

#endif // !_HX_WHEN_ANY_H_