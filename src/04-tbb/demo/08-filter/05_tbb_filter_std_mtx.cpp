#include <vector>
#include <mutex>
#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 27;
    std::vector<float> arr;
    std::mutex mtx;
    {
        HX::STL::utils::TickTock<> _{"05_tbb_filter_std_mtx"};
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            std::vector<float> tmpArr;
            tmpArr.reserve(r.size());
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (float v = std::sin(i); v > 0) {
                    tmpArr.push_back(v);
                }
            }
            std::lock_guard _{mtx};
            std::copy(tmpArr.begin(), tmpArr.end(), std::back_inserter(arr));
        });
    }
    return 0;
}