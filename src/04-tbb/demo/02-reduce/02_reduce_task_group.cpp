#include <cmath>
#include <vector>
#include <tbb/task_group.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    float res = 0;
    {
        tbb::task_group tg;
        size_t maxt = 16;
        std::vector<float> arr(maxt);
        HX::STL::utils::TickTock<> _{"reduce_task_group"};
        for (size_t t = 0; t < maxt; ++t) {
            size_t begin = t * n / maxt, end = std::min(n, (t + 1) * n / maxt);
            tg.run([&, begin, end, t] {
                float ans = 0;
                for (size_t i = begin; i < end; ++i) {
                    ans += std::sin(i);
                }
                arr[t] = ans;
            });
        }
        tg.wait();
        for (auto v : arr)
            res += v;
    }
    HX::print::println("res: ", res);
    return 0;
}