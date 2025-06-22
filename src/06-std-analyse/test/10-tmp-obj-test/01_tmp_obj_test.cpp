#include <HXprint/print.h>

using namespace HX;

struct PrintMan {
    void man() {
        print::println("man!");
    }

    PrintMan&& set() && {
        return std::move(*this);
    }

    ~PrintMan() noexcept {
        print::println("~PrintMan");
    }
};

struct A {
    PrintMan func(int) && {
        print::println("todo...");
        return {};
    }

    auto cb() && {
        return [this] {
            return std::move(*this).func(0);
        };
    }

    static PrintMan fk(A&& a) {
        print::println("fate...");
        return {};
    }

    PrintMan&& build() && {
        return PrintMan{}.set();
    }

    ~A() noexcept {
        print::println("~A");
    }
};

A makeA() {
    return A{};
}

void link(PrintMan pm) {
    pm.man();
}

int main() {
    A{}.func(0).man();
/*
    auto res = co_await AioTask::linkTimeout(
        _iocp.makeAioTask().prepRead(
            hStdin, buf, 0
        ),
        _iocp.makeAioTask().prepLinkTimeout(
            hStdin, makeTimer().sleepFor(3s)
        )
    );
*/
    print::println("==============");
    A::fk(A{}).man();
    print::println("==============");
    makeA().cb()().man();
    // 生命周期都在一行内, 不管是 move 到内部了还是怎么样, 根生命周期应该都是在这一行!
    print::println("==============");
    link(makeA().build());
    print::println("==============");
    return 0;
}