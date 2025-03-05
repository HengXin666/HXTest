#include <deque>
#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#include <tbb/co>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 1 << 10;
    {
        tbb::concurrent_vector<float> arr;
        std::vector<float*> ptrArr(n);
        {
            HX::STL::utils::TickTock<> _{"tbb::concurrent_vector 推入元素"};
            for (size_t i = 0; i < n; ++i) {
                ptrArr[i] = &*arr.push_back(0);
            }
        }
        for (size_t i = 0; i < n; ++i) {
            HX::print::print(int(ptrArr[i] == &arr[i]), ' ');
        }
    }
    HX::print::println("\n");
    // 注意 std::deque<T> 也是链式扩容的 (准确的说是 一块 ->(链)-> 一块)
    {
        std::deque<float> arr;
        std::vector<float*> ptrArr(n);
        {
            HX::STL::utils::TickTock<> _{"std::deque 推入元素"};
            for (size_t i = 0; i < n; ++i) {
                arr.push_back(0);
                ptrArr[i] = &arr.back();
            }
        }
        for (size_t i = 0; i < n; ++i) {
            HX::print::print(int(ptrArr[i] == &arr[i]), ' ');
        }
    }
    HX::print::println("\ngrow_by:");
    {
        tbb::concurrent_vector<float> arr;
        for (size_t i = 0; i < 10; ++i) {
            auto it = arr.grow_by(2);
            *it++ = 1;
            *it++ = 2;
        }
        HX::print::println(arr);
    }

    tbb::concurrent_vector<float> arr;
    HX::print::println("可安全地被多线程并发访问:");
    {
        tbb::parallel_for((size_t)0, (size_t)n, [&](size_t i) {
            auto it = arr.grow_by(2);
            *it++ = std::sin(i);
            *it++ = std::cos(i);
        });
        HX::print::println("size: ", arr.size());
    }
    HX::print::println("不建议通过索引随机访问:");
    {
        HX::STL::utils::TickTock<> _{"索引访问"};
        for (size_t i = 0; i < arr.size(); ++i) {
            arr[i] = i;
        }
    }
    {
        HX::STL::utils::TickTock<> _{"迭代器 + i 访问"};
        for (size_t i = 0; i < arr.size(); ++i) {
            *(arr.begin() + i) = i;
        }
    }
    {
        HX::STL::utils::TickTock<> _{"迭代器 访问"};
        size_t i = 0;
        for (auto& v : arr) {
            v = i++;
        }
    }
    {
        tbb::parallel_for(tbb::blocked_range(arr.begin(), arr.begin()),
        [&](tbb::blocked_range<decltype(arr.begin())> r) {
            for (auto it = r.begin(); it != r.end(); ++it) {
                *it += 1.0f;
            }
        });
        HX::print::println(arr[1]);
    }
    return 0;
}