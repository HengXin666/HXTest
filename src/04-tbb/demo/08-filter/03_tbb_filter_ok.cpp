#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_vector.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 27;
    tbb::concurrent_vector<float> arr;
    {
        HX::STL::utils::TickTock<> _{"03-tbb-filter-ok"};
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            std::vector<float> tmpArr;
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (float v = std::sin(i); v > 0) {
                    tmpArr.push_back(v);
                }
            }
            auto it = arr.grow_by(tmpArr.size());
            for (size_t i = 0; i < tmpArr.size(); ++i) {
                *it++ = tmpArr[i];
            }
        });
    }
    return 0;
}