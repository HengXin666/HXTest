#include <tbb/parallel_invoke.h>

#include <HXSTL/utils/TickTock.hpp>

int serial_fib(int n) {
    return n < 2 ? n : serial_fib(n - 1) + serial_fib(n - 2);
}

int fib(int n) {
    if (n < 29)
        return serial_fib(n);
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
        HX::STL::utils::TickTock<> _{"04_tbb_scheduling_overhead_fib"};
        HX::print::println(fib(39));
    }
    return 0;
}