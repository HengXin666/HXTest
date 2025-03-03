#include <vector>
#include <cmath>
#include <tbb/task_group.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    std::vector<float> arr(n);
    tbb::task_group tg;
    size_t maxt = 32;
    {
        HX::STL::utils::TickTock<> _("map_task_group");
        for (size_t t = 0; t < maxt; ++t) {
            size_t begin = t * n / maxt, end = std::min(n, (t + 1) * n / maxt);
            tg.run([&, begin, end] {
                for (size_t i = begin; i < end; ++i) {
                    arr[i] = std::sin(i);
                }
            });
        }
        tg.wait();
    }
    return 0;
}