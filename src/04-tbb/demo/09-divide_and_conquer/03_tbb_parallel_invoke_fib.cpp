#include <tbb/parallel_invoke.h>

#include <HXSTL/utils/TickTock.hpp>

int fib(int n) {
    if (n < 2)
        return n;
    int a, b;
    tbb::parallel_invoke([&]{
        a = fib(n - 1);
    }, [&]{
        b = fib(n - 2);
    });
    return a + b;
}

int main() {
    {
        HX::STL::utils::TickTock<> _{"03_tbb_parallel_invoke_fib"};
        HX::print::println(fib(39));
    }
    return 0;
}