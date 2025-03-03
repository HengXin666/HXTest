#include <vector>
#include <cmath>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    std::vector<float> arr(n);
    {
        HX::STL::utils::TickTock<> _("map_parallel_for (可矢量优化)");
        tbb::parallel_for(tbb::blocked_range<size_t>{0, n},
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                arr[i] = std::sin(i);
            }
        });
    }
    {
        HX::STL::utils::TickTock<> _("map_parallel_for (萌新)");
        tbb::parallel_for((size_t)0, (size_t)n, [&](size_t i) {
            arr[i] = std::sin(i);
        });
    }
    {
        HX::STL::utils::TickTock<> _("map_parallel_for_each (萌新)");
        tbb::parallel_for_each(arr.begin(), arr.end(), [&](float& v) {
            v = 32.f;
        });
    }
    return 0;
}