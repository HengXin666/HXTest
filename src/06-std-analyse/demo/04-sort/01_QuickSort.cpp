#include <HXprint/print.h>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>

#include <algorithm>

// 版本
#define SORT_V 0x05

namespace hx {

namespace internal {

#if SORT_V == 0x01
struct Sorter {
    void sort(int begin, int end) {
        if (begin >= end) {
            return;
        }
        auto l = begin, r = end;
        auto t = std::move(arr[l]);
        bool runLight = false;
        while (l < r) {
            if (runLight) {
                if (arr[l] > t) {
                    std::swap(arr[l], arr[r]);
                    runLight ^= 1;
                } else {
                    ++l;
                }
            } else {
                if (arr[r] < t) {
                    std::swap(arr[l], arr[r]);
                    runLight ^= 1;
                } else {
                    --r;
                }
            }
        }
        arr[l] = std::move(t);
        sort(begin, l - 1);
        sort(l + 1, end);
    }
    std::vector<int>& arr;
};
#elif SORT_V == 0x02
struct Sorter {
    void sort(int begin, int end) {
        if (begin >= end) {
            return;
        }
        auto l = begin, r = end;
        auto t = std::move(arr[l]);
        while (l < r) {
            while (l < r && arr[r] >= t) // 反人类的从 r 开始
                --r;
            while (l < r && arr[l] <= t) 
                ++l;
            if (l < r)
                std::swap(arr[l], arr[r]);
        }
        std::swap(arr[begin], arr[l]);
        sort(begin, l - 1);
        sort(l + 1, end);
    }
    std::vector<int>& arr;
};
#elif SORT_V == 0x03
struct Sorter {
    // 单边循环法, 交换次数更少, 也更简单
    void sort(int begin, int end) {
        if (begin >= end) {
            return;
        }
        auto j = begin;
        auto& t = arr[j];
        for (auto i = begin + 1; i <= end; ++i) {
            if (arr[i] < t)
                std::swap(arr[++j], arr[i]);
        }
        std::swap(arr[begin], arr[j]);
        sort(begin, j - 1);
        sort(j + 1, end);
    }
    std::vector<int>& arr;
};
#elif SORT_V == 0x04
template <typename T>
struct Sorter {
    // 支持迭代器
    template <typename It>
    void sort(const It begin, const It end) {
        if (std::distance(begin, end) <= 1) {
            return;
        }
        auto j = begin;
        auto& t = *j;
        for (auto i = begin + 1; i != end; ++i) {
            if (*i < t)
                std::swap(*++j, *i);
        }
        std::swap(*begin, *j);
        sort(begin, j);
        sort(j + 1, end);
    }
    T& arr;
};
#elif SORT_V == 0x05
template <typename T, typename Func>
struct Sorter {
    // 支持谓词 和 甚至 std::forward_list 的 仅前向自增迭代器
    template <typename It>
    void sort(const It begin, const It end) {
        if (begin == end) {
            return;
        }
        auto j = begin;
        auto& t = *j;
        for (auto i = [&] { auto res = begin; return ++res; }(); i != end; ++i) {
            if (func(*i, t))
                std::swap(*++j, *i);
        }
        std::swap(*begin, *j);
        sort(begin, j);
        sort(++j, end);
    }
    T& arr;
    const Func& func;
};
#endif

} // namespace internal

template <typename T, typename Func>
auto sort(T& arr, const Func& func) -> decltype(
    arr.begin(), arr.end(),
    std::is_same_v<bool, decltype(func(*arr.begin(), *arr.begin()))>, void()
) {
    internal::Sorter {arr, func}.sort(arr.begin(), arr.end());
}

template <typename T>
auto sort(T& arr) -> decltype(
    *arr.begin() < *arr.begin(), void()
) {
    using Type = std::decay_t<decltype(*arr.begin())>;
    internal::Sorter {arr, [](const Type& i, const Type& j) {
        return i < j;
    }}.sort(arr.begin(), arr.end());
}

} // namespace hx

int main() {
    std::forward_list<int> arr{1, 4, 3, 3, 2, 2, 3};
    HX::print::println(arr);
    hx::sort(arr, [](int i, int j) {
        return i > j;
    });
    HX::print::println(arr);
    hx::sort(arr);
    HX::print::println(arr);
    return 0;
}