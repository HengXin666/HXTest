#include <ranges>
#include <HXTest.hpp>

namespace HX::ranges {

template <typename T>
concept Range = requires(T& t) {
    std::ranges::begin(t);
    std::ranges::end(t);
};

template <typename T>
using RangeType = decltype(*std::ranges::begin(std::declval<T&>())); // 注意必须是 左值引用

template <typename F, typename Arg>
concept Pred = requires(F&& func, Arg&& arg) {
    { std::forward<F>(func)(std::forward<Arg>(arg)) } -> std::convertible_to<bool>;
};

template <typename Crtp>
    requires (std::is_object_v<Crtp> && std::same_as<Crtp, std::remove_cvref_t<Crtp>>)
struct ViewInterface {
    constexpr auto& base() noexcept {
        return derived()._base;
    }
    constexpr auto& base() const noexcept {
        return derived()._base;
    }
    constexpr auto data() noexcept {
        return std::to_address(std::ranges::begin(base()));
    }
    constexpr auto data() const noexcept {
        return std::to_address(std::ranges::begin(base()));
    }
    constexpr bool empty() const noexcept {
        return std::ranges::end(base()) - std::ranges::begin(base());
    }
    constexpr std::size_t size() const noexcept {
        return std::ranges::end(base()) - std::ranges::begin(base());
    }
private:
    constexpr Crtp& derived() noexcept {
        static_assert(std::derived_from<Crtp, ViewInterface>);
        return static_cast<Crtp&>(*this);
    }
    constexpr Crtp const& derived() const noexcept {
        static_assert(std::derived_from<Crtp, ViewInterface>);
        return static_cast<Crtp const&>(*this);
    }
};

// F = [](RangeType<R>) -> bool {}
template <Range R, Pred<RangeType<R>> P>
struct FilterView : public ViewInterface<FilterView<R, P>> {
    FilterView() requires (std::default_initializable<R>
			      && std::default_initializable<P>)
	= default;

    constexpr FilterView(R range, P pred) 
        : _base{std::move(range)}
        , _pred{std::move(pred)}
    {}

    constexpr auto begin() {
        // @todo: cache 首次查找后可以缓存, 日后无需再查找
        return std::ranges::find_if(_base, _pred);
    }
    constexpr auto end() {
        return std::ranges::end(_base);
    }
private:
    R _base{};
    P _pred;

    friend ViewInterface<FilterView<R, P>>;
};

template <Range R, Pred<RangeType<R>> P>
FilterView(R&&, P) -> FilterView<std::views::all_t<R>, P>;

namespace views {

namespace internal {

template <typename Derived, typename... Args>
concept IsMemoOpCall = requires {
    std::declval<Derived>()(std::declval<Args>()...);
};

template <typename Derived, typename... Args>
struct MemoOp {
    std::tuple<Args...> _tp;
    
    template <typename... Ts>
    constexpr MemoOp(int, Ts&&... ts)
        : _tp{std::forward<Ts>(ts)...}
    {}

    template <Range R>
        requires (IsMemoOpCall<Derived, R, Args...>)
    constexpr auto operator()(R&& range) && noexcept {
        return std::apply([&range](auto&... args) {
            return Derived{}(std::forward<R>(range), std::move(args)...);
        }, _tp);
    }

    template <Range R>
        requires (IsMemoOpCall<Derived, R, Args const&...>)
    constexpr auto operator()(R&& range) const& noexcept {
        return std::apply([&range](auto const&... args) {
            return Derived{}(std::forward<R>(range), args...);
        }, _tp);
    }

    // 为了避免 std::move(const obj) 这种使用, 在编译期报出
    template <Range R>
    constexpr auto operator()(R&&) const&& noexcept = delete;
};

template <typename Derived, typename... Args>
concept IsOpMakeOp = Derived::_OpArgCnt > 1
    && Derived::_OpArgCnt - 1 == sizeof...(Args)
    && (std::constructible_from<std::decay_t<Args>, Args> && ...);

template <typename Derived>
struct OpMake {
    template <typename... Args>
        requires (IsOpMakeOp<Derived, Args...>)
    constexpr auto operator()(Args&&... args) const {
        return MemoOp<Derived, std::decay_t<Args>...>{0, std::forward<Args>(args)...};
    }
};

} // namespace internal

struct FilterOp : public internal::OpMake<FilterOp> {
    template <typename R, typename P>
        // @todo requires 是否可以被 FilterView 调用
    [[nodiscard]] constexpr auto operator()(R&& range, P&& pred) const {
        return FilterView{std::forward<R>(range), std::forward<P>(pred)};
    }

    using internal::OpMake<FilterOp>::operator();
    inline static constexpr int _OpArgCnt = 2;
};

inline constexpr FilterOp filter{};

template <Range R, typename Self>
    // @todo requires 是否可以被调用 && Self 的类型概念
constexpr auto operator|(R&& range, Self&& self) {
    return std::forward<Self>(self)(std::forward<R>(range));
}

} // namespace views

} // namespace HX::ranges

namespace ranges = HX::ranges;
namespace views = HX::ranges::views;

void test() {
    auto filter = std::ranges::filter_view{std::vector<int>{1,3,3,2,5}, [](auto) {
        return true;
    }};
    [[maybe_unused]] auto wdf = filter.begin();
    for (auto v : filter) {
        (void)v;   
    }
    auto view = std::vector<int>{1,3,3,2,5} | std::views::filter([](auto) {
        return true;
    });
    auto arr = std::vector<int>{22, 33};
    auto view2 = arr | std::views::filter([](auto) {
        return true;
    });
    static_assert(!std::same_as<decltype(view), decltype(view2)>, "sb");
    auto v3 = std::views::filter(std::vector<int>{22, 33}, [](auto) {
        return true;
    });
}

int main() {
    std::vector<int> arr{1,3,3,2,5};
    auto all = std::views::all(arr);
    all.base();
    auto filter = ranges::FilterView{arr, [](auto v) {
        log::hxLog.warning(v);
        return true;
    }};
    auto filter2 = ranges::FilterView{std::vector<int>{}, [](auto) { return true; }};
    [[maybe_unused]] auto wdf = filter.base();
    for (auto v : filter) {
        log::hxLog.info(v);
    }
    for (auto it = filter.begin(); it != filter.end(); ++it) {
        log::hxLog.info(*it);
    }
    // 目前是 只有一个传入 range
    // 选定一个 定制点对象
    // 先进行 lbd 传入, 等操作
    // 才进行 op| 的传入
    // 说明: filter(lbd) -> 新的状态的一个对象
    // 何时进行编译期判断? 在组装的 op| 中, 以 range 作为标准
    auto view = std::vector<int>{2, 2, 3, 4} | views::filter([](auto) {
        return true;
    });
    auto view2 = arr | views::filter([](auto) {
        return true;
    });
    static_assert(!std::same_as<decltype(view), decltype(view2)>, "sb");
    for (auto v : view2) {
        log::hxLog.error(v);
    }
}