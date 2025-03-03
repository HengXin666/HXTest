#include <vector>
#include <cmath>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    std::vector<float> arr(n);
    float res = 0;
    {
        HX::STL::utils::TickTock<> _{"scan_std"};
        for (size_t i = 0; i < n; ++i) {
            res += std::sin(i);
            arr[i] = res;
        }
    }
    HX::print::println("arr[n/2]: ", arr[n/2]);
    HX::print::println("res: ", res);
    return 0;
}