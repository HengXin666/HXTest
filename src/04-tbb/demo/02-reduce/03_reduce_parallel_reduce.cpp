#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    float res;
    {
        HX::STL::utils::TickTock<> _("reduce_parallel_reduce");
        res = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, n), (float)0,
        [&](tbb::blocked_range<size_t> r, float v) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                v += std::sin(i);
            }
            return v;
        }, [](float a, float b) {
            return a + b;
        });
    }
    HX::print::println("res: ", res);
    {
        HX::STL::utils::TickTock<> _("reduce_parallel_deterministic_reduce");
        res = tbb::parallel_deterministic_reduce(tbb::blocked_range<size_t>(0, n), (float)0,
        [&](tbb::blocked_range<size_t> r, float v) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                v += std::sin(i);
            }
            return v;
        }, [](float a, float b) {
            return a + b;
        });
    }
    HX::print::println("res: ", res);
    return 0;
}