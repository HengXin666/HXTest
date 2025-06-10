#include <HXprint/print.h>

#include <memory>
#include <tools/NonVoidHelper.hpp>

namespace HX {

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
            return std::move(UninitializedNonVoidVariantImpl<Idx + 1, Ts...>::template get<Index>(std::move(v._body)));
        }
    }
};

} // namespace internal

inline constexpr std::size_t UninitializedNonVoidVariantNpos = static_cast<std::size_t>(-1);

template <typename... Ts>
struct UninitializedNonVoidVariant {
    inline static constexpr std::size_t N = sizeof...(Ts);

    std::size_t index() const noexcept {
        return _idx;
    }

    template <std::size_t Idx>
        requires (Idx <= N)
    constexpr auto& get() & noexcept {
        _idx = Idx;
        return internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(_data);
    }

    template <std::size_t Idx>
        requires (Idx <= N)
    constexpr auto&& get() && noexcept {
        _idx = UninitializedNonVoidVariantNpos;
        return std::move(internal::UninitializedNonVoidVariantImpl<0, Ts...>::template get<Idx>(std::move(_data)));
    }

    // @todo
    // template <typename T, typename... Args>
    // T emplace(Args&&... args) {
    //     new (std::addressof(_data)) T(std::forward<Args>(args)...);
    //     return _data;
    // }
    
private:
    internal::UninitializedNonVoidVariantImpl<0, Ts...> _data;
    std::size_t _idx{UninitializedNonVoidVariantNpos};
};

template <size_t Idx, typename... Ts,
    typename = std::enable_if_t<(Idx <= UninitializedNonVoidVariant<Ts...>::N)>>
auto& get(UninitializedNonVoidVariant<Ts...>& v) noexcept {
    return v.template get<Idx>();
}

template <size_t Idx, typename... Ts,
    typename = std::enable_if_t<(Idx <= UninitializedNonVoidVariant<Ts...>::N)>>
auto&& get(UninitializedNonVoidVariant<Ts...>&& v) noexcept {
    return std::move(std::move(v).template get<Idx>());
}

} // namespace HX

int main() {
    using namespace HX;
    UninitializedNonVoidVariant<int, char, char, char, char> awa;
    auto& res = get<0>(awa);
    res = 0x7F'FF'FF'FF;
    (void)awa;
    HX::print::println("get: ", (unsigned char)get<3>(awa));
    return 0;
}