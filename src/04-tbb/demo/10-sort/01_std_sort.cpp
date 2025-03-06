#include <algorithm>
#include <vector>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 22;
    std::vector<int> arr(n);
    std::generate(arr.begin(), arr.end(), std::rand);
    {
        HX::STL::utils::TickTock<> _{"01_std_sort"};
        std::sort(arr.begin(), arr.end(), std::less<>{});
    }
    return 0;
}