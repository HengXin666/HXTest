#include <HXprint/print.h>

// 返回值测试 1
auto __test01__ = []{
    HX::print::println("Test01 {");
    struct A {
        A&& todo() && {
            return std::move(*this);
        }

        static A&& mk() {
            return std::move(A{}.todo());
        }

        ~A() noexcept {
            HX::print::println("~A ", this);
        }
    };
    {
        auto&& a = A::mk();
        (void)a;
        HX::print::println("--- mk End a ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    {
        decltype(auto) b = A::mk();
        (void)b;
        HX::print::println("--- mk End b ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    {
        auto c = A::mk(); // 返回的 A&& 通过移动构造出新的的 A; 而不是延续之前的 A&&
        (void)c;
        HX::print::println("--- mk End c ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    {
        auto func = [](A a) {
            (void)a;
            HX::print::println("func\n");
            return;
        };
        func(A::mk()); // 返回的 A&& 通过移动构造出新的的 A; 而不是延续之前的 A&&
        HX::print::println("--- mk End d ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    HX::print::println("} // Test01");
    return 0;
}();

#include <chrono>
#include <optional>
#include <map>

// 返回值测试 2
auto __test02__ = []{
    HX::print::println("Test02 {");
    struct A {
        A&& todo() && {
            return std::move(*this);
        }

        static A&& mk() {
            return std::move(A{}.todo());
        }

        ~A() noexcept {
            HX::print::println("~A ", this);
            (void)_p;
        }

    private:
        using MdTree = std::multimap<std::chrono::system_clock::time_point, int>;
        std::optional<MdTree::iterator> _p{};
    };
    struct ABuild {
        A&& build() && {
            return std::move(A{}.todo());
        }
        ~ABuild() noexcept {
            HX::print::println("~ABuild ", this);
        }
    };
    auto makeBuild = []{
        return ABuild{};
    };
    {
        auto&& a = makeBuild().build();
        (void)a;
        HX::print::println("--- mk End a ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    {
        decltype(auto) b = makeBuild().build();
        (void)b;
        HX::print::println("--- mk End b ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    {
        auto c = makeBuild().build();
        (void)c;
        HX::print::println("--- mk End c ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    {
        auto func = [](A a) {
            (void)a;
            HX::print::println("func\n");
            return;
        };
        func(makeBuild().build()); // ub
                                   // 调试的时候就可以发现:
                                   // return std::move(A{}.todo());
                                   // 发生了析构, 才进行返回!
                                   // 因此访问的是悬挂引用!
        HX::print::println("--- mk End d ---\n");
    }
    HX::print::println("--- --- --- --- ---\n");
    HX::print::println("} // Test02");
    return 0;
}();

int main() {
    return 0;
}