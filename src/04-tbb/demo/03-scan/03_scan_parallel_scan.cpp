#include <cmath>
#include <vector>
#include <tbb/blocked_range.h>
#include <tbb/parallel_scan.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 26;
    std::vector<float> arr(n);
    float res = 0;
    {
        HX::STL::utils::TickTock<> _{"scan_parallel_scan"};
        res = tbb::parallel_scan(tbb::blocked_range<size_t>(0, n), (float)0,
        [&](tbb::blocked_range<size_t> r, float local_res, auto is_final) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                local_res += std::sin(i);
                if (is_final) {
                    arr[i] = local_res;
                }
            }
            return local_res;
        }, [](float a, float b) {
            return a + b;
        });
    }
    HX::print::println("arr[n/2]: ", arr[n/2]);
    HX::print::println("res: ", res);
    return 0;
}
