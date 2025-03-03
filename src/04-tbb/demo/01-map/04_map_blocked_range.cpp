#include <vector>
#include <cmath>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <tbb/blocked_range3d.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 13;
    std::vector<float> arr(n * n);
    {
        HX::STL::utils::TickTock<> _{"map_blocked_range2d"};
        tbb::parallel_for(tbb::blocked_range2d<size_t>(0, n, 0, n),
        [&](tbb::blocked_range2d<size_t> r) {
            for (size_t i = r.cols().begin(); i < r.cols().end(); ++i) {
                for (size_t j = r.rows().begin(); j < r.rows().end(); ++j) {
                    arr[i * n + j] = std::sin(i) * std::sin(j);
                }
            }
        });
    }
    {
        n = 1000;
        arr.resize(n * n * n);
        HX::STL::utils::TickTock<> _{"map_blocked_range3d"};
        tbb::parallel_for(tbb::blocked_range3d<size_t>(0, n, 0, n, 0, n),
        [&](tbb::blocked_range3d<size_t> r) {
            for (size_t i = r.pages().begin(); i < r.pages().end(); ++i) {
                for (size_t j = r.cols().begin(); j < r.cols().end(); ++j) {
                    for (size_t k = r.rows().begin(); k < r.rows().end(); ++k) {
                        arr[(i * n + j) * n + k] = std::sin(i) * std::sin(j) * std::sin(k);
                    }
                }
            }
        });
    }
    return 0;
}