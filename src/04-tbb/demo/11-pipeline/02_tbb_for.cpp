#include <vector>
#include <cmath>
#include <tbb/parallel_for_each.h>

#include <HXSTL/utils/TickTock.hpp>

struct Data {
    std::vector<float> arr;

    Data() {
        arr.resize(std::rand() % 100 * 500 + 10000);
        for (size_t i = 0; i < arr.size(); i++) {
            arr[i] = std::rand() * (1.f / (float)RAND_MAX);
        }
    }

    void step1() {
        for (size_t i = 0; i < arr.size(); i++) {
            arr[i] += 3.14f;
        }
    }

    void step2() {
        std::vector<float> tmp(arr.size());
        for (size_t i = 1; i < arr.size() - 1; i++) {
            tmp[i] = arr[i - 1] + arr[i] + arr[i + 1];
        }
        std::swap(tmp, arr);
    }

    void step3() {
        for (size_t i = 0; i < arr.size(); i++) {
            arr[i] = std::sqrt(std::abs(arr[i]));
        }
    }

    void step4() {
        std::vector<float> tmp(arr.size());
        for (size_t i = 1; i < arr.size() - 1; i++) {
            tmp[i] = arr[i - 1] - 2 * arr[i] + arr[i + 1];
        }
        std::swap(tmp, arr);
    }
};

int main() {
    size_t n = 1 << 12;
    std::vector<Data> dats(n);
    {
        HX::STL::utils::TickTock<> _{"02_tbb_for"};
        tbb::parallel_for_each(dats.begin(), dats.end(), [&](Data& dat) {
            dat.step1();
            dat.step2();
            dat.step3();
            dat.step4();
        });
    }
    return 0;
}
