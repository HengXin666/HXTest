#include <vector>
#include <cmath>
#include <mutex>
#include <tbb/parallel_for.h>
#include <tbb/task_arena.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 13;
    std::vector<float> arr(n * n);
    std::mutex mtx;
    tbb::parallel_for((size_t)0, (size_t)n, [&](size_t i) {
        std::lock_guard _{mtx};
        // 用 isolate 隔离, 禁止其内部的工作被窃取
        tbb::this_task_arena::isolate([&] {
            tbb::parallel_for((size_t)0, (size_t)n, [&](size_t j) {
                arr[i * n + j] = std::sin(i) * std::sin(j);
            });
        });
    });
    return 0;
}