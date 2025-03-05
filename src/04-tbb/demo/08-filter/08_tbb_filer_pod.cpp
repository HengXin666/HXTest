#include <vector>
#include <atomic>
#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

template <class T>
struct pod {
private:
    T _t;

public:
    pod() {}

    pod(pod &&p) : _t(std::move(p._t)) {}

    pod(pod const &p) : _t(p._t) {}

    pod &operator=(pod &&p) {
        _t = std::move(p._t);
        return *this;
    }

    pod &operator=(pod const &p) {
        _t = p._t;
        return *this;
    }

    pod(T &&t) : _t(std::move(t)) {}

    pod(T const &t) : _t(t) {}

    pod &operator=(T &&t) {
        _t = std::move(t);
        return *this;
    }

    pod &operator=(T const &t) {
        _t = t;
        return *this;
    }

    operator T const &() const {
        return _t;
    }

    operator T &() {
        return _t;
    }

    T const &get() const {
        return _t;
    }

    T &get() {
        return _t;
    }

    template <class... Ts>
    pod &emplace(Ts &&...ts) {
        ::new (&_t) T(std::forward<Ts>(ts)...);
        return *this;
    }

    void destroy() {
        _t.~T();
    }
};

int main() {
    size_t n = 1 << 27;
    std::vector<pod<float>> arr;
    arr.reserve(n); // 预留空间
    std::atomic<size_t> arrSize = 0;
    {
        HX::STL::utils::TickTock<> _{"08_tbb_filter_pod"};
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            std::vector<pod<float>> tmpArr(r.size());
            size_t tmpArrSize = 0;
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (float v = std::sin(i); v > 0) {
                    tmpArr[tmpArrSize++] = v;
                }
            }
            size_t base = arrSize.fetch_add(tmpArrSize);
            for (size_t i = 0; i < tmpArrSize; ++i) {
                arr[base++] = tmpArr[i];
            }
        });
    }
    return 0;
}