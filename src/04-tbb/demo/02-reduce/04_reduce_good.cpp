#include <cmath>
#include <vector>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    // init
    size_t n = 1 << 26;
    std::vector<float> arr(n);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = 10.f + std::sin(i);
    }

    float sum = 0;
    {
        HX::STL::utils::TickTock<> _{"朴素求平均值(浮点有误差)"};
        for (size_t i = 0; i < n; ++i) {
            sum += arr[i];
        }
    }
    HX::print::println("朴素求平均值(浮点有误差): ", sum / n);

    {
        HX::STL::utils::TickTock<> _{"并行缩并求平均值(浮点几乎无误差)"};
        sum = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, n), (float)0,
        [&](tbb::blocked_range<size_t> r, float v) {
            for (size_t i = r.begin(); i < r.end(); ++i)
                v += arr[i];
            return v;
        }, [](float a, float b) {
            return a + b;
        });
    }
    HX::print::println("并行缩并求平均值(浮点几乎无误差): ", sum / n);
    return 0;
}