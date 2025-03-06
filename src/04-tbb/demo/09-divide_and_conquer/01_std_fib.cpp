#include <HXSTL/utils/TickTock.hpp>

int fib(int n) {
    if (n < 2)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main() {
    {
        HX::STL::utils::TickTock<> _{"01_std_fib"};
        HX::print::println(fib(39));
    }
    return 0;
}