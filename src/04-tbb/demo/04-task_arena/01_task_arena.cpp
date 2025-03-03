#include <vector>
#include <cmath>
#include <tbb/task_arena.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 13;
    std::vector<float> arr(n);
    {
        tbb::task_arena ta;
        ta.execute([&] {
            tbb::parallel_for((size_t)0, (size_t)n, [&](size_t i) {
                arr[i] = std::sin(i);
            });
        });
    }
    {
        tbb::task_arena ta(4); // 指定使用的线程数
        ta.execute([&] {
            tbb::parallel_for((size_t)0, (size_t)n, [&](size_t i) {
                arr[i] = std::sin(i);
            });
        });
    }
    return 0;
}