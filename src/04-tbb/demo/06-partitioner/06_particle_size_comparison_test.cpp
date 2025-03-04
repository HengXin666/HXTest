#include <vector>
#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    std::vector<float> arr(n);
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::simple_partitioner: 粒度为 1 太细了, 效果不好"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
            [&](tbb::blocked_range<size_t> r) {
                for (size_t i = r.begin(); i < r.end(); ++i) {
                    arr[i] = std::sin(i);
                }
            }, tbb::simple_partitioner{});
        });
    }
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::static_partitioner: 粒度为 n / 4, 效果好"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
            [&](tbb::blocked_range<size_t> r) {
                for (size_t i = r.begin(); i < r.end(); ++i) {
                    arr[i] = std::sin(i);
                }
            }, tbb::static_partitioner{});
        });
    }
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::simple_partitioner: 粒度手动设为 n / 8, 效果稍微更好一点"};
        ta.execute([&]{
            int c = ta.max_concurrency();
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n, n / (2 * c)),
            [&](tbb::blocked_range<size_t> r) {
                for (size_t i = r.begin(); i < r.end(); ++i) {
                    arr[i] = std::sin(i);
                }
            }, tbb::simple_partitioner{});
        });
    }
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::auto_partitioner: 自动判断也不错"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
            [&](tbb::blocked_range<size_t> r) {
                for (size_t i = r.begin(); i < r.end(); ++i) {
                    arr[i] = std::sin(i);
                }
            }, tbb::auto_partitioner{});
        });
    }
    return 0;
}