#include <vector>
#include <cmath>
#include <mutex>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 13;
    std::vector<float> arr(n * n);
    // std::mutex mtx; // 这个会死锁, 因为 tbb 是任务窃取的, 因此一个线程可能重复上锁!
    std::recursive_mutex mtx;
    tbb::parallel_for((size_t)0, (size_t)n, [&](size_t i) {
        std::lock_guard _{mtx};
        tbb::parallel_for((size_t)0, (size_t)n, [&](size_t j) {
            arr[i * n + j] = std::sin(i) * std::sin(j);
        });
    });
    return 0;
}