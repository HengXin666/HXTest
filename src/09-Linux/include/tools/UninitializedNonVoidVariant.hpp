#pragma once
/*
 * Copyright Heng_Xin. All rights reserved.
 *
 * @Author: Heng_Xin
 * @Date: 2025-06-10 22:37:12
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
#ifndef _HX_UNINITIALIZED_NON_VOID_VARIANT_H_
#define _HX_UNINITIALIZED_NON_VOID_VARIANT_H_

#include <utility>
#include <memory>

#include <tools/NonVoidHelper.hpp>
#include <tools/ExceptionMode.hpp>

namespace HX {

template <typename... Ts>
struct UninitializedNonVoidVariant;

namespace internal {

template <std::size_t Idx, typename T>
struct UninitializedNonVoidVariantHead {
    NonVoidType<T> _data;
};

template <std::size_t Idx, typename... Ts>
struct UninitializedNonVoidVariantImpl;

template <std::size_t Idx, typename T>
struct UninitializedNonVoidVariantImpl<Idx, T> {
    using UVariant = UninitializedNonVoidVariantImpl;

    union {
        UninitializedNonVoidVariantHead<Idx, T> _head;
    };

    template <std::size_t Index>
    inline static constexpr NonVoidType<T>& get(UVariant& v) noexcept {
        return v._head._data;
    }

    template <std::size_t Index>
    inline static constexpr auto&& get(UVariant&& v) noexcept {
        return std::move(v._head._data);
    }
};

template <std::size_t Idx, typename T, typename... Ts>
struct UninitializedNonVoidVariantImpl<Idx, T, Ts...> {
    using UVariant = UninitializedNonVoidVariantImpl;

    union {
        UninitializedNonVoidVariantHead<Idx, T> _head;
        UninitializedNonVoidVariantImpl<Idx + 1, Ts...> _body;
    };

    template <std::size_t Index>
    inline static constexpr auto& get(UVariant& v) noexcept {
        if constexpr (Index == Idx) {
            return v._head._data;
        } else {
            return UninitializedNonVoidVariantImpl<Idx + 1, Ts...>::template get<Index>(v._body);
        }
    }

    template <std::size_t Index>
    inline static constexpr auto&& get(UVariant&& v) noexcept {
        if constexpr (Index == Idx) {
            return std::move(v._head._data);
        } else {
            return std::move(
                UninitializedNonVoidVariantImpl<Idx + 1, Ts...>::template get<Index>(std::move(v._body))
            );
        }
    }
};

template <typename T, typename... Ts>
struct UninitializedNonVoidVariantIndex;

template <typename T, typename... Ts>
struct UninitializedNonVoidVariantIndex<T, UninitializedNonVoidVariant<Ts...>> {
    template <typename U, typename... Us>
    struct _FindIndex;
    
    template <typename U, typename... Us>
    struct _FindIndex {
        inline static constexpr std::size_t val 
            = std::is_same_v<T, U> 
                ? 0 
                : UninitializedNonVoidVariantIndex<
                    T, UninitializedNonVoidVariant<Ts...>
                  >::_FindIndex<Us...>::val + 1;
    };

    template <typename U>
    struct _FindIndex<U> {
        inline static constexpr std::size_t val 
            = !std::is_same_v<T, U>;
    };

    inline static constexpr std::size_t val
        = UninitializedNonVoidVariantIndex<
            T, UninitializedNonVoidVariant<Ts...>
          >::_FindIndex<Ts...>::val;
};

} // namespace internal

inline constexpr std::size_t UninitializedNonVoidVariantNpos
    = static_cast<std::size_t>(-1);

// 如果不存在, 则返回 UVariant::N
template <typename T, typename UVariant>
inline constexpr std::size_t UninitializedNonVoidVariantIndexVal
    = internal::UninitializedNonVoidVariantIndex<T, UVariant>::val;

/**
 * @brief 一个支持任意类型的 Variant, 内部类型可以重复, 支持 void, 访问不受约束 (日后可能会支持约束版本)
 *        get<Idx>(v) = xxx; // 内部索引会直接变, 调用者需要保证类型本身支持 = 运算符.
 * @tparam ...Ts
 */
template <typename... Ts>
struct UninitializedNonVoidVariant {
    inline static constexpr std::size_t N = sizeof...(Ts);

    UninitializedNonVoidVariant() noexcept
        : _idx{UninitializedNonVoidVariantNpos}
    {}

    template <typename T>
    explicit UninitializedNonVoidVariant(T&& t) noexcept(std::is_nothrow_constructible_v<T, T&&>)
        : _idx{UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>}
    {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        new (std::addressof(get<Idx, ExceptionMode::Nothrow>())) T(std::forward<T>(t));
    }

    std::size_t index() const noexcept {
        return _idx;
    }

    template <std::size_t Idx, ExceptionMode EMode = ExceptionMode::Throw>
        requires (Idx < N)
    constexpr auto& get() & noexcept(EMode == ExceptionMode::Nothrow) {
        if constexpr (EMode == ExceptionMode::Throw) {
            if (_idx != Idx) [[unlikely]] {
                throw std::runtime_error("get: wrong index for variant");
            }
        }
        return internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(_data);
    }

    template <std::size_t Idx, ExceptionMode EMode = ExceptionMode::Throw>
        requires (Idx < N)
    constexpr auto&& get() && noexcept(EMode == ExceptionMode::Nothrow) {
        if constexpr (EMode == ExceptionMode::Throw) {
            if (_idx != Idx) [[unlikely]] {
                throw std::runtime_error("get: wrong index for variant");
            }
        }
        return std::move(
            internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(std::move(_data))
        );
    }

    /**
     * @brief 获取类型为 T 的变量; 如果多个类型, 仅会提取最靠前的那个
     * @tparam T 
     */
    template <typename T, ExceptionMode EMode = ExceptionMode::Throw>
        requires (UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant> < N)
    constexpr auto& get() & noexcept(EMode == ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return get<Idx, EMode>();
    }

    template <typename T, ExceptionMode EMode = ExceptionMode::Throw>
        requires (UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant> < N)
    constexpr auto&& get() && noexcept(EMode == ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return std::move(std::move(*this).template get<Idx, EMode>());
    }

    template <typename T, typename... Args>
    T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        del(std::make_index_sequence<N>{});
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        _idx = Idx;
        new (std::addressof(get<Idx, ExceptionMode::Nothrow>())) T(std::forward<Args>(args)...);
        return get<Idx, ExceptionMode::Nothrow>();
    }

    void reset() noexcept {
        del(std::make_index_sequence<N>{});
        _idx = UninitializedNonVoidVariantNpos;
    }
    
    ~UninitializedNonVoidVariant() noexcept {
        del(std::make_index_sequence<N>{});
    }

private:
    template <std::size_t... Idx>
    void del(std::index_sequence<Idx...>) noexcept {
        using DelFuncPtr = void (*)(UninitializedNonVoidVariant& u);
        static DelFuncPtr delFuncs[N] {
            [](UninitializedNonVoidVariant& u){ u.get<Idx, ExceptionMode::Nothrow>().~Ts(); }...
        };
        if (_idx != UninitializedNonVoidVariantNpos) {
            delFuncs[_idx](*this);
        }
    }

    std::size_t _idx;

    union {
        internal::UninitializedNonVoidVariantImpl<0, Ts...> _data;
    };
};

template <std::size_t Idx, typename... Ts, 
    ExceptionMode EMode = ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx <= UninitializedNonVoidVariant<Ts...>::N)>>
auto& get(UninitializedNonVoidVariant<Ts...>& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return v.template get<Idx, EMode>();
}

template <std::size_t Idx, typename... Ts, 
    ExceptionMode EMode = ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx <= UninitializedNonVoidVariant<Ts...>::N)>>
auto&& get(UninitializedNonVoidVariant<Ts...>&& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return std::move(std::move(v).template get<Idx, EMode>());
}

template <typename T, typename... Ts,
    ExceptionMode EMode = ExceptionMode::Throw>
auto& get(UninitializedNonVoidVariant<Ts...>& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return v.template get<T, EMode>();
}

template <typename T, typename... Ts,
    ExceptionMode EMode = ExceptionMode::Throw>
auto&& get(UninitializedNonVoidVariant<Ts...>&& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return std::move(std::move(v).template get<T, EMode>());
}

} // namespace HX

#endif // !_HX_UNINITIALIZED_NON_VOID_VARIANT_H_