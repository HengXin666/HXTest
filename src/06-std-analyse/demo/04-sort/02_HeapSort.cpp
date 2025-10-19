#include <HXTest.hpp>

#include <vector>

namespace HX {

template <
    typename T,
    typename Cmp = std::less<T>,
    typename ArrType = std::vector<T>
>
class PriorityQueue {
public:
    using Type = T;
    using ArrayType = ArrType;
    using CmpType = Cmp;

    PriorityQueue() {
        _arr.resize(1); // 第一个不用
    }

    template <typename U>
        requires (std::convertible_to<U, T>)
    void push(U&& u) {
        // 放入尾部, 然后不断比对上浮
        _arr.push_back(std::move(u));
        for (auto idx = _arr.size() - 1; idx > 1 && _cmp(_arr[idx], _arr[idx >> 1]); idx >>= 1) {
            std::swap(_arr[idx], _arr[idx >> 1]);
        }
    }

    void pop() {
        if (_arr.size() <= 1) [[unlikely]] {
            throw std::out_of_range{"pop"};
        }
        // 1. 头和尾交换
        std::swap(_arr[1], _arr.back());
        // 2. 尾部 pop
        _arr.pop_back();
        // 3. 头部下沉
        for (std::size_t idx = 1; ; ) {
            auto left = idx << 1;
            auto right = left | 1;
            if (left >= _arr.size()) {
                break;
            }
            auto next = left;
            // 选择更 优 的子节点 进行交换
            if (right < _arr.size() && _cmp(_arr[right], _arr[left])) {
                next = right;
            }
            if (_cmp(_arr[idx], _arr[next])) {
                break;
            }
            std::swap(_arr[idx], _arr[next]);
            idx = next;
        }
    }

    T& top() {
        if (_arr.size() <= 1) [[unlikely]] {
            throw std::out_of_range{"top"};
        }
        return _arr[1];
    }

    T const& top() const {
        if (_arr.size() <= 1) [[unlikely]] {
            throw std::out_of_range{"top"};
        }
        return _arr[1];
    }

    std::size_t size() const noexcept {
        return _arr.size() - 1;
    }
private:

    /*
            i / 2
          i       ~
    2 * i, 2 * i + 1
    */
    ArrayType _arr{};
    CmpType _cmp{};
};

template <typename T, typename Cmp = std::less<T>>
constexpr void heapSort(std::vector<T>& arr, Cmp cmp = Cmp{}) {
    auto n = arr.size();
    if (n < 2) 
        return;

    // 下沉函数, 注意这里排序规则写法和之前不同.
    // 因为堆实际上是删除元素, 作为排序完成的. 所以 堆顶 对应 排序后数组末尾
    auto siftDown = [&](std::size_t idx, std::size_t end) -> void {
        /*
            从 0 开始作为索引, 其映射规律:
            left(i)   = 2 * i + 1
            right(i)  = 2 * i + 2
            parent(i) = (i - 1) / 2
        */
        for (;;) {
            std::size_t left = (idx << 1) | 1;
            if (left >= end) 
                break;
            std::size_t right = left + 1;
            std::size_t best = left;
            if (right < end && cmp(arr[left], arr[right])) {
                best = right;
            }
            if (!cmp(arr[idx], arr[best]))
                break;
            std::swap(arr[idx], arr[best]);
            idx = best;
        }
    };

    // 1. 建堆 (从最后一个非叶节点开始下沉)
    for (std::size_t i = (n >> 1); i-- > 0; ) {
        // 注意! 仅有父节点会影响堆的性质.
        // 为何是非叶子? 因为叶子没有儿子, 下沉个寂寞!
        siftDown(i, n);
    }

    // 2. 排序: 交换堆顶与末尾, 缩小堆范围, 再下沉
    for (std::size_t end = n; end > 1;) {
        std::swap(arr[0], arr[--end]);
        siftDown(0, end);
    }
}

} // namespace HX

int main() {
    using namespace HX;

    PriorityQueue<int, decltype([](auto a, auto b) {
        return a > b;
    })> pq;
    pq.push(3);
    pq.push(3);
    pq.push(3);
    pq.push(3);
    pq.push(1);
    pq.push(2);
    pq.push(2);

    for (; pq.size(); pq.pop()) {
        log::hxLog.info(pq.top());
    }

    std::vector<int> arr{1,4,3,3,2,2,3};
    log::hxLog.info(arr);
    heapSort(arr);
    log::hxLog.info(arr);
    return 0;
}