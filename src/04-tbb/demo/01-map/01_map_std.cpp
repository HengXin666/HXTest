#include <vector>
#include <cmath>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    int n = 1 << 26;
    std::vector<float> arr(n);
    {
        HX::STL::utils::TickTock<> _("map_std");
        for (size_t i = 0; i < arr.size(); ++i) {
            arr[i] = std::sin(i);
        }
    }
    return 0;
}