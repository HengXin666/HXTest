#include <HXprint/print.h>

#if 0
struct A {
    auto req() {
        return _req(); // Function '_req' with deduced return type cannot be used before it is defined
    }

private:
    auto _req() {
        return 2233;
    }
};
#endif

struct AInTwoFind {
    template <class T>
    auto req() {
        return _req<T>(); // ok
    }

private:
    template <class>
    auto _req() {
        return 2233;
    }
};

int main() {
    AInTwoFind a2;
    a2.req<int>();
    return 0;
}