#include <cmath>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    float res = 0;
    {
        HX::STL::utils::TickTock<> _{"reduce_std"};
        for (size_t i = 0; i < n; ++i) {
            res += std::sin(i);
        }
    }
    HX::print::println("res: ", res);
    return 0;
}