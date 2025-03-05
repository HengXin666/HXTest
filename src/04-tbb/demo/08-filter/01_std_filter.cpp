#include <vector>
#include <cmath>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 27;
    std::vector<float> arr;
    {
        HX::STL::utils::TickTock<> _{"01-stl-filter"};
        for (size_t i = 0; i < n; ++i) {
            if (float v = std::sin(i); v > 0) {
                arr.push_back(v);
            }
        }
    }
    return 0;
}