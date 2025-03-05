#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_vector.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 27;
    tbb::concurrent_vector<float> arr;
    {
        HX::STL::utils::TickTock<> _{"02-tbb-filter-err"};
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (float v = std::sin(i); v > 0) {
                    arr.push_back(v);
                }
            }
        });
    }
    return 0;
}