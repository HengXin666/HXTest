#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 32;
    using namespace std::chrono;
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::static_partitioner 用于循环体不均匀的情况效果不好"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
            [&](tbb::blocked_range<size_t> r) {
                HX::print::println("线程: ", tbb::this_task_arena::current_thread_index(), ", size: ", r.size());
                for (size_t i = r.begin(); i < r.end(); ++i) {
                    std::this_thread::sleep_for(i * 10ms);
                }
            }, tbb::static_partitioner{});
        });
    }
    {
        tbb::task_arena ta(4);
        HX::STL::utils::TickTock<> _{"tbb::simple_partitioner 用于循环体不均匀的情况效果很好"};
        ta.execute([&]{
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
            [&](tbb::blocked_range<size_t> r) {
                HX::print::println("线程: ", tbb::this_task_arena::current_thread_index(), ", size: ", r.size());
                for (size_t i = r.begin(); i < r.end(); ++i) {
                    std::this_thread::sleep_for(i * 10ms);
                }
            }, tbb::simple_partitioner{});
        });
    }
    return 0;
}