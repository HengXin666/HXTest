#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <HXSTL/utils/TickTock.hpp>

int main() {
    size_t n = 32;
    using namespace std::chrono;
    {
        tbb::task_arena ta(4);
        tbb::affinity_partitioner affinity;
        for (size_t t = 0; t < 10; ++t) {
            HX::STL::utils::TickTock<> _{"tbb::affinity_partitioner: " + std::to_string(t)};
            ta.execute([&]{
                tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
                [&](tbb::blocked_range<size_t> r) {
                    for (size_t i = r.begin(); i < r.end(); ++i) {
                        for (volatile size_t j = 0; j < i * 1000; ++j)
                            ;
                    }
                }, affinity);
            });
        }
    }
    return 0;
}