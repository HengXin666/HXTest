#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 32;
    {
        tbb::task_arena ta(4);
        /*
            创建了 4 个线程 32 个任务
            每个任务包含 1 个元素
        */
        HX::STL::utils::TickTock<> _{"auto_partitioner"};
        ta.execute([&]{
            using namespace std::chrono;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
            [&](tbb::blocked_range<size_t> r) {
                HX::print::println("线程: ", tbb::this_task_arena::current_thread_index(), ", size: ", r.size());
                std::this_thread::sleep_for(400ms);
            }, tbb::auto_partitioner{});
        });
    }
    return 0;
}