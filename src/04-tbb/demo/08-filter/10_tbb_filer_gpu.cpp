#include <vector>
#include <cmath>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_scan.h>

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
    std::vector<pod<float>> arr(n);
    std::vector<pod<size_t>> ind(n + 1);
    {
        HX::STL::utils::TickTock<> _{"10_tbb_filter_gpu"};
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                arr[i] = std::sin(i);
            }
        });
        ind[0] = 0;
        tbb::parallel_scan(tbb::blocked_range<size_t>(0, n), (size_t)0,
        [&](tbb::blocked_range<size_t> r, size_t sum, auto is_final) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                sum += arr[i] > 0 ? 1 : 0;
                if (is_final) {
                    ind[i + 1] = sum;
                }
            }
            return sum;
        }, [](size_t x, size_t y) {
            return x + y;
        });
        std::vector<pod<float>> res(ind.back());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (arr[i] > 0) {
                    res[ind[i]] = arr[i];
                }
            }
        });
    }
    return 0;
}