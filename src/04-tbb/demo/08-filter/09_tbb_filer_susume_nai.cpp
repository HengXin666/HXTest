#include <vector>
#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 27;
    {
        HX::STL::utils::TickTock<> _{"09_tbb_filter_不推荐"};
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, n), std::vector<float>{},
        [&](tbb::blocked_range<size_t> r, std::vector<float> local_arr) {
            local_arr.reserve(local_arr.size() + r.size());
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (float v = std::sin(i); v > 0) {
                    local_arr.push_back(v);
                }
            }
            return local_arr;
        }, [] (std::vector<float> a, std::vector<float> const& b) {
            std::copy(b.begin(), b.end(), std::back_inserter(a));
            return a;
        });
    }
    return 0;
}