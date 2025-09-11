#include <HXLibs/log/Log.hpp>

template <typename Func>
void callFunc(Func func) { // 如果写 Func&& 导致外部被推导为 Func&
                           // 从而资源会被 std::move 导致全部窃取... 原本期望应该是拷贝, 而不是移动 なのに
                           // https://github.com/HengXin666/HXLibs/commit/e8153a067874baeacefdfd51e8491c30d9802c69
    [_cb = std::move(func)] {
        _cb();
    }();
}

template <std::size_t... N, typename Func>
void add(Func func) {
    static_assert(sizeof...(N) > 0, "");
    if constexpr (sizeof...(N) == 1) {
        callFunc(std::move(func));
    } else {
        (callFunc(((void)N, func)), ...);
    }
}

int main() {
    auto ptr = std::make_shared<bool>();
    add<0>([=]{
        if (ptr) {
            HX::log::hxLog.info("ok");
        } else {
            HX::log::hxLog.error("err!");
        }
    });
    add<0, 1, 2, 3>([=]{
        if (ptr) {
            HX::log::hxLog.info("ok");
        } else {
            HX::log::hxLog.error("err!");
        }
    });
    return 0;
}