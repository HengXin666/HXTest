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
    inline static constexpr const NonVoidType<T>& get(UVariant const& v) noexcept {
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
    inline static constexpr auto& get(UVariant const& v) noexcept {
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
            = std::is_same_v<T, NonVoidType<U>> 
                ? 0 
                : UninitializedNonVoidVariantIndex<
                    T, UninitializedNonVoidVariant<Ts...>
                  >::_FindIndex<Us...>::val + 1;
    };

    template <typename U>
    struct _FindIndex<U> {
        inline static constexpr std::size_t val 
            = !std::is_same_v<T, NonVoidType<U>>;
    };

    inline static constexpr std::size_t val
        = UninitializedNonVoidVariantIndex<
            T, UninitializedNonVoidVariant<Ts...>
          >::_FindIndex<Ts...>::val;
};

template <std::size_t Idx, typename... Ts>
struct UninitializedNonVoidVariantIndexToType;

template <std::size_t Idx, typename T, typename... Ts>
    requires (Idx <= sizeof...(Ts))
struct UninitializedNonVoidVariantIndexToType<Idx, UninitializedNonVoidVariant<T, Ts...>> {
    template <typename U, typename... Us>
    struct IndexToType {
        using Type = std::conditional_t<
            (Idx == UninitializedNonVoidVariantIndex<
                    NonVoidType<U>, UninitializedNonVoidVariant<T, Ts...>
            >::val),
            NonVoidType<U>,
            typename IndexToType<Us...>::Type
        >;
    };

    template <typename U>
    struct IndexToType<U> {
        using Type = NonVoidType<U>;
    };

    using Type = IndexToType<T, Ts...>::Type;
};

} // namespace internal

inline constexpr std::size_t UninitializedNonVoidVariantNpos
    = static_cast<std::size_t>(-1);

// 如果不存在, 则返回 UVariant::N
template <typename T, typename UVariant>
inline constexpr std::size_t UninitializedNonVoidVariantIndexVal
    = internal::UninitializedNonVoidVariantIndex<T, UVariant>::val;

// 通过 Idx, 返回 UVariant 对应位置的类型
template <std::size_t Idx, typename... Ts>
using UninitializedNonVoidVariantIndexToType
    = internal::UninitializedNonVoidVariantIndexToType<
        Idx, UninitializedNonVoidVariant<Ts...>>::Type;

/**
 * @brief 一个支持任意类型的 Variant, 内部类型可以重复, 支持 void
 * @tparam ...Ts
 */
template <typename... Ts>
struct UninitializedNonVoidVariant {
    inline static constexpr std::size_t N = sizeof...(Ts);

    constexpr UninitializedNonVoidVariant() noexcept
        : _idx{UninitializedNonVoidVariantNpos}
    {}

    template <typename T>
    explicit UninitializedNonVoidVariant(T&& t) noexcept(std::is_nothrow_constructible_v<T, T&&>)
        : _idx{UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>}
    {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        new (std::addressof(get<Idx, ExceptionMode::Nothrow>())) T(std::forward<T>(t));
    }

    UninitializedNonVoidVariant(UninitializedNonVoidVariant const& that) 
        : UninitializedNonVoidVariant{}
    {
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            return;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        that.get<Idx, ExceptionMode::Nothrow>()
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
    }

    UninitializedNonVoidVariant(UninitializedNonVoidVariant&& that) 
        : UninitializedNonVoidVariant{}
    {
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            return;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        std::move(std::move(that).template get<Idx, ExceptionMode::Nothrow>())
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
    }

    constexpr std::size_t index() const noexcept {
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
    constexpr auto& get() const & noexcept(EMode == ExceptionMode::Nothrow) {
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
    constexpr const auto& get() const & noexcept(EMode == ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return get<Idx, EMode>();
    }

    template <typename T, ExceptionMode EMode = ExceptionMode::Throw>
        requires (UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant> < N)
    constexpr auto&& get() && noexcept(EMode == ExceptionMode::Nothrow) {
        constexpr std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>;
        return std::move(std::move(*this).template get<Idx, EMode>());
    }

    template <typename T, typename... Args,
        std::size_t Idx = UninitializedNonVoidVariantIndexVal<T, UninitializedNonVoidVariant>>
            requires (Idx < N)
    T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        del(std::make_index_sequence<N>{});
        _idx = Idx;
        new (std::addressof(get<Idx, ExceptionMode::Nothrow>())) T(std::forward<Args>(args)...);
        return get<Idx, ExceptionMode::Nothrow>();
    }

    template <std::size_t Idx, typename... Args,
        typename T = UninitializedNonVoidVariantIndexToType<Idx, Ts...>>
            requires (Idx < N)
    T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        del(std::make_index_sequence<N>{});
        _idx = Idx;
        new (std::addressof(get<Idx, ExceptionMode::Nothrow>())) T(std::forward<Args>(args)...);
        return get<Idx, ExceptionMode::Nothrow>();
    }

    template <typename T>
        requires (!std::is_same_v<std::decay_t<T>, UninitializedNonVoidVariant>)
    UninitializedNonVoidVariant& operator=(T&& t) noexcept {
        emplace<T>(std::forward<T>(t));
        return *this;
    }

    UninitializedNonVoidVariant& operator=(UninitializedNonVoidVariant const& that) noexcept {
        if (std::addressof(that) == this) {
            return *this;
        }
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            reset();
            return *this;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        that.get<Idx, ExceptionMode::Nothrow>()
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
        return *this;
    }

    constexpr UninitializedNonVoidVariant& operator=(UninitializedNonVoidVariant&& that) noexcept {
        if (std::addressof(that) == this) {
            return *this;
        }
        if (that._idx == UninitializedNonVoidVariantNpos) [[unlikely]] {
            reset();
            return *this;
        }
        [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
            (((that._idx == Idx) && (
                [&] {
                    emplace<Idx>(
                        std::move(std::move(that).template get<Idx, ExceptionMode::Nothrow>())
                    );
                    return true;
                }()
            )) || ...);
        } (std::make_index_sequence<N>{});
        return *this;
    }

    constexpr bool operator==(UninitializedNonVoidVariant const& that) noexcept {
        if (_idx == that._idx) {
            if (_idx == UninitializedNonVoidVariantNpos) {
                return true;
            }
            bool res = false;
            [&] <std::size_t... Idx> (std::index_sequence<Idx...>) {
                (((_idx == Idx) && [&] {
                    res = (get<Idx>() == that.get<Idx>());
                    return true;
                }()) || ...);
            } (std::make_index_sequence<N>{});
            return res;
        }
        return false;
    }

    constexpr bool operator!=(UninitializedNonVoidVariant const& that) noexcept {
        return !(*this == that);
    }

    void reset() noexcept {
        del(std::make_index_sequence<N>{});
    }
    
    ~UninitializedNonVoidVariant() noexcept {
        del(std::make_index_sequence<N>{});
    }

private:
    template <std::size_t... Idx>
    void del(std::index_sequence<Idx...>) noexcept {
        using DelFuncPtr = void (*)(UninitializedNonVoidVariant&);
        static DelFuncPtr delFuncs[N] {
            [](UninitializedNonVoidVariant& u){ u.get<Idx, ExceptionMode::Nothrow>().~NonVoidType<Ts>(); }...
        };
        if (_idx != UninitializedNonVoidVariantNpos) {
            delFuncs[_idx](*this);
            _idx = UninitializedNonVoidVariantNpos;
        }
    }

    std::size_t _idx;

    union {
        internal::UninitializedNonVoidVariantImpl<0, Ts...> _data;
    };
};

template <std::size_t Idx, typename... Ts, 
    ExceptionMode EMode = ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx < UninitializedNonVoidVariant<Ts...>::N)>>
constexpr auto& get(UninitializedNonVoidVariant<Ts...>& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return v.template get<Idx, EMode>();
}

template <std::size_t Idx, typename... Ts, 
    ExceptionMode EMode = ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx < UninitializedNonVoidVariant<Ts...>::N)>>
constexpr const auto& get(UninitializedNonVoidVariant<Ts...> const& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return v.template get<Idx, EMode>();
}

template <std::size_t Idx, typename... Ts, 
    ExceptionMode EMode = ExceptionMode::Throw,
    typename = std::enable_if_t<(Idx < UninitializedNonVoidVariant<Ts...>::N)>>
constexpr auto&& get(UninitializedNonVoidVariant<Ts...>&& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return std::move(std::move(v).template get<Idx, EMode>());
}

template <typename T, typename... Ts,
    ExceptionMode EMode = ExceptionMode::Throw>
constexpr auto& get(UninitializedNonVoidVariant<Ts...>& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return v.template get<T, EMode>();
}

template <typename T, typename... Ts,
    ExceptionMode EMode = ExceptionMode::Throw>
constexpr const auto& get(UninitializedNonVoidVariant<Ts...> const& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return v.template get<T, EMode>();
}

template <typename T, typename... Ts,
    ExceptionMode EMode = ExceptionMode::Throw>
constexpr auto&& get(UninitializedNonVoidVariant<Ts...>&& v) noexcept(EMode == ExceptionMode::Nothrow) {
    return std::move(std::move(v).template get<T, EMode>());
}

template <typename Lambda, typename... Ts, 
    typename Res = decltype(std::declval<Lambda>()(
        get<0>(std::declval<UninitializedNonVoidVariant<Ts...>>())))>
constexpr Res visit(UninitializedNonVoidVariant<Ts...>& uv, Lambda&& lambda) {
    if constexpr (std::is_void_v<Res>) {
        auto fun = [&] <std::size_t Idx> (std::index_sequence<Idx>) {
            lambda(uv.template get<Idx, ExceptionMode::Nothrow>());
            return true;
        };
        [&] <std::size_t... Idx>(std::index_sequence<Idx...>) {
            ((uv.index() == Idx && fun(std::index_sequence<Idx>{})) || ...);
        }(std::make_index_sequence<sizeof...(Ts)>{});
        return;
    } else {
        Res res;
        auto fun = [&] <std::size_t Idx> (std::index_sequence<Idx>) {
            res = lambda(uv.template get<Idx, ExceptionMode::Nothrow>());
            return true;
        };
        [&] <std::size_t... Idx>(std::index_sequence<Idx...>) {
            ((uv.index() == Idx && fun(std::index_sequence<Idx>{})) || ...);
        }(std::make_index_sequence<sizeof...(Ts)>{});
        return res;
    }
}

} // namespace HX

#endif // !_HX_UNINITIALIZED_NON_VOID_VARIANT_H_