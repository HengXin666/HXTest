#include <vector>
#include <cmath>
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 14;
    std::vector<float> arr(n * n);
    std::vector<float> brr(n * n);
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::auto_partitioner: 默认矩阵转置"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range2d<size_t>(0, n, 0, n),
            [&](tbb::blocked_range2d<size_t> r) {
                for (size_t i = r.cols().begin(); i < r.cols().end(); ++i) {
                    for (size_t j = r.rows().begin(); j < r.rows().end(); ++j) {
                        brr[i * n + j] = arr[j * n + i];
                    }
                }
            }, tbb::auto_partitioner{});
        });
    }
    {
        tbb::task_arena ta(4);
        size_t grain = 16;
        HX::STL::utils::TickTock<> _{"tbb::auto_partitioner: 默认矩阵转置"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range2d<size_t>(0, n, grain, 0, n, grain),
            [&](tbb::blocked_range2d<size_t> r) {
                for (size_t i = r.cols().begin(); i < r.cols().end(); ++i) {
                    for (size_t j = r.rows().begin(); j < r.rows().end(); ++j) {
                        brr[i * n + j] = arr[j * n + i];
                    }
                }
            }, tbb::simple_partitioner{});
        });
    }
    return 0;
}