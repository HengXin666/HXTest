#include <vector>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 10;
    {
        std::vector<float> arr;
        std::vector<float*> ptrArr(n);
        for (size_t i = 0; i < n; ++i) {
            arr.push_back(0);
            ptrArr[i] = &arr.back();
        }
        for (size_t i = 0; i < n; ++i) {
            HX::print::print(ptrArr[i] == &arr[i], ' ');
        }
    }
    HX::print::println("\n");
    {
        std::vector<float> arr;
        std::vector<float*> ptrArr(n);
        arr.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            arr.push_back(0);
            ptrArr[i] = &arr.back();
        }
        for (size_t i = 0; i < n; ++i) {
            HX::print::print(ptrArr[i] == &arr[i], ' ');
        }
    }
    return 0;
}