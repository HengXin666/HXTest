#include <tbb/task_group.h>

#include <HXSTL/utils/TickTock.hpp>

int fib(int n) {
    if (n < 2)
        return n;
    int a, b;
    tbb::task_group tg;
    tg.run([&]{
        a = fib(n - 1);
    });
    tg.run([&]{
        b = fib(n - 2);
    });
    tg.wait();
    return a + b;
}

int main() {
    {
        HX::STL::utils::TickTock<> _{"02_tbb_task_fib"};
        HX::print::println(fib(39));
    }
    return 0;
}