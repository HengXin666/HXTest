#include <HXTest.hpp>

using namespace HX;

struct A {
    struct B {
        B(A&& a) {
            std::move(a).sb();
            log::hxLog.warning("B");
        }

        void loli() {
            log::hxLog.debug("todo");
        }

        ~B() {
            log::hxLog.warning("~B");
        }
    };

    B await() && {
        return {std::move(*this)};
    }

    ~A() {
        log::hxLog.info("~A");
    }

    void sb() && {}
};


int main() {
    std::move(A{}).await().loli();
    return 0;
}