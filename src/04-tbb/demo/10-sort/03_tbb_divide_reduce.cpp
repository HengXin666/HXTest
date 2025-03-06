#include <cstdlib>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <tbb/parallel_invoke.h>

#include <HXSTL/utils/TickTock.hpp>

template <class T>
T quick_reduce(T *data, size_t size) {
    if (size < (1<<16)) {
        return std::reduce(data, data + size);
    }
    T sum1, sum2;
    size_t mid = size / 2;
    tbb::parallel_invoke([&] {
        sum1 = quick_reduce(data, mid);
    }, [&] {
        sum2 = quick_reduce(data + mid, size - mid);
    });
    return sum1 + sum2;
}

int main() {
    size_t n = 1<<25;
    std::vector<int> arr(n);
    int sum;
    std::generate(arr.begin(), arr.end(), std::rand);
    {
        HX::STL::utils::TickTock<> _{"03_tbb_divide_reduce"};
        sum = quick_reduce(arr.data(), arr.size());
    }
    printf("%d\n", sum);
    return 0;
}
