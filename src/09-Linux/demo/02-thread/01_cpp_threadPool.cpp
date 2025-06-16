#include <HXprint/print.h>
#include <tools/Uninitialized.hpp>
#include <thread>
#include <functional>
#include <queue>
#include <map>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

/**
 * @brief 线程安全的队列
 * @tparam T 
 */
template <typename T>
struct SafeQueue {
    SafeQueue()
        : _queue{}
        , _mtx{}
    {}

    SafeQueue(SafeQueue const&) = delete;
    SafeQueue& operator=(SafeQueue const&) = delete;

    decltype(auto) front() const {
        std::shared_lock _{_mtx};
        return _queue.front();
    }

    void pop() {
        std::unique_lock _{_mtx};
        _queue.pop();
    }

    T frontAndPop() {
        std::unique_lock _{_mtx};
        auto&& res = std::move(_queue.front());
        _queue.pop();
        return res;
    }

    template <typename... Args>
    decltype(auto) emplace(Args&&... args) {
        std::unique_lock _{_mtx};
        return _queue.emplace(std::forward<Args>(args)...);
    }

    std::size_t size() const {
        std::shared_lock _{_mtx};
        return _queue.size();
    }

    bool empty() const {
        std::shared_lock _{_mtx};
        return _queue.empty();
    }
private:
    std::queue<T> _queue;
    mutable std::shared_mutex _mtx;
};

template <typename T>
struct Result {
    using FutureResult = HX::NonVoidType<T>;

    Result() {}

    Result& operator=(Result&&) = default;
    Result(Result&&) = default;

    Result(Result const&) = delete;
    Result& operator=(Result const&) = delete;

    FutureResult get() noexcept {
        return _res;
    }

    FutureResult& getFutureResult() noexcept {
        return _res;
    }
private:
    FutureResult _res;
};

/**
 * @brief 线程池数据
 */
struct ThreadPoolData {
    uint32_t minThreadNum;      // 最小线程数
    uint32_t maxThreadNum;      // 最大线程数

    uint32_t taskNum;           // (未执行的)任务数
    uint32_t nowThreadNum;      // 当前线程数

    uint32_t runThreadNum;      // 运行的线程数
    uint32_t sleepThreadNum;    // 挂起的线程数
};

inline auto ThreadPoolDefaultStrategy = [](ThreadPoolData const& data) -> int {
    if (data.taskNum <= data.minThreadNum) {
        if (data.sleepThreadNum > data.runThreadNum) {
            return -static_cast<int>(std::max<std::size_t>(
                static_cast<std::size_t>(data.sleepThreadNum * 0.25),
                data.nowThreadNum - data.minThreadNum
            ));
        }
        return 0;
    }
    return static_cast<int>(std::min<std::size_t>(
        data.maxThreadNum - data.nowThreadNum,
        static_cast<std::size_t>(
            data.sleepThreadNum ? 0 : 2
        ) 
    ));
};

struct ThreadPool {
    enum class Model {
        Synchronous,
        Asynchronous
    };

    ThreadPool()
        : _taskQueue{}
        , _opThread{}
        , _workers{}
        , _delIdQueue{}
        , _cv{}
        , _mtx{}
        , _minThreadNum{1}
        , _maxThreadNum{std::thread::hardware_concurrency()}
        , _runCnt{0}
        , _delCnt{0}
        , _isRun{false}
    {}

    /**
     * @brief 添加任务, 任务应当为右值传入
     * @tparam Func 右值传入
     * @tparam Args 如果为左值, 请自行权衡生命周期, 以防止悬垂引用
     * @param func 
     * @param args 
     */
    template <typename Func, typename... Args, typename Res = std::invoke_result_t<Func, Args...>>
    Result<Res> addTask(Func&& func, Args&&... args) {
        Result<Res> res;
        auto& ans = res.getFutureResult();
        _taskQueue.emplace([&, func = std::move(func)] {
            if constexpr (std::is_void_v<Res>) {
                static_cast<void>(ans);
                func(std::forward<Args>(args)...);
            } else {
                ans.set(func(std::forward<Args>(args)...));
            }
        });
        _cv.notify_one();
        return res;
    }

    /**
     * @brief 启动线程池
     */
    template <Model Md = Model::Asynchronous, typename Strategy>
    void run(std::chrono::milliseconds checkTimer, Strategy strategy) {
        _isRun = true;
        if constexpr (Md == Model::Asynchronous) {
            _opThread = std::thread{&ThreadPool::check<decltype(strategy)>,
                                    this, checkTimer, std::move(strategy)};
        } else if constexpr (Md == Model::Synchronous) {
            check(checkTimer, std::move(strategy));
        } else {
            // 内部错误
            static_assert(sizeof(Md) < 0, "Error");
        }
    }

    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool() noexcept {
        _isRun = false;
        HX::print::println("ThreadPool End~");
        _opThread.join();
        _cv.notify_all();
        for (const auto& [_, t] : _workers) {
            t->join();
        }
        if (!_delIdQueue.empty()) {
            clearDelIdQueue();
        }
        HX::print::println("ThreadPool sa yo na ra!");
    }
private:
    /**
     * @brief 管理者检查线程
     * @param checkTimer 检查 间隔的时间 (ms)
     */
    template <typename Strategy>
    void check(std::chrono::milliseconds checkTimer, Strategy&& strategy) {
        if (_workers.size() < _minThreadNum) {
            // init
            makeWorker(_minThreadNum - _workers.size());
        }
        while (_isRun) [[likely]] {
            HX::print::println("check~");
            auto taskSize = static_cast<uint32_t>(_taskQueue.size());
            auto runCnt = _runCnt.load();
            if (int add = strategy(ThreadPoolData{
                _minThreadNum,
                _maxThreadNum,
                taskSize,
                static_cast<uint32_t>(_workers.size()),
                runCnt,
                taskSize - runCnt
            })) {
                if (add > 0) {
                    makeWorker(static_cast<std::size_t>(add));
                } else {
                    add = -add;
                    _delCnt = static_cast<uint32_t>(add);
                    for (int i = 0; i < add; --i) {
                        _cv.notify_one();
                    }
                }
            }
            if (_delIdQueue.size()) {
                clearDelIdQueue();
            }
            std::this_thread::sleep_for(checkTimer);
        }
    }

    /**
     * @brief 创建生产者
     * @param num 创建的数量
     */
    void makeWorker(std::size_t num) {
        for (std::size_t i = 0; i < num; ++i) {
            auto up = std::make_unique<std::thread>([this] {
                HX::print::print("启动: ");
                std::cout << std::this_thread::get_id() << '\n';
                while (true) {
                    HX::print::println("我先睡了~");
                    {
                        // 挂起, 并等待任务
                        std::unique_lock lck{_mtx};
                        _cv.wait(lck, [&] {
                            return !_taskQueue.empty() || !_isRun || _delCnt > 0;
                        });
                    }
                    if (!_isRun) [[unlikely]] {
                        break;
                    }
                    if (tryDecrementIfPositive(_delCnt)) {
                        _delIdQueue.emplace(std::this_thread::get_id());
                        break;
                    }
                    ++_runCnt;
                    HX::print::println("上班~");
                    auto task = _taskQueue.frontAndPop();
                    try {
                        task(); // 执行任务
                    } catch (...) {
                        ;
                    }
                    --_runCnt;
                }
            });
            auto id = up->get_id();
            _workers.emplace(id, std::move(up));
        }
    }

    /**
     * @brief 从待删除的队列中取出id, 并且从红黑树中删除
     */
    void clearDelIdQueue() {
        while (!_delIdQueue.empty()) {
            auto id = _delIdQueue.frontAndPop();
            auto it = _workers.find(id);
            it->second->join();
            _workers.erase(it);
        }
    }

    /**
     * @brief 如果 val > 0, 则减小其值; CAS, 整个过程是原子的
     * @param val 
     * @return true  执行成功
     * @return false 执行失败, 不满足 val > 0
     */
    bool tryDecrementIfPositive(std::atomic_uint32_t& val) {
        uint32_t now = val.load(std::memory_order_relaxed);
        while (now > 0) {
            // 尝试原子替换 now -> now - 1
            if (val.compare_exchange_weak(
                now,                // 当前期望值
                now - 1,            // 设置的新值
                std::memory_order_acquire,
                std::memory_order_relaxed
            )) {
                return true;
            }
            // compare_exchange_weak 修改了 now
            // 循环重试直到 now <= 0 或替换成功
        }
        return false; // now <= 0
    }

    // 任务队列
    SafeQueue<std::function<void()>> _taskQueue;

    // 管理者线程
    std::thread _opThread;

    // 消费者线程
    std::map<std::thread::id, std::unique_ptr<std::thread>> _workers;

    // 待删除队列
    SafeQueue<std::thread::id> _delIdQueue;

    // 消费者的信号量 & 对应的互斥锁
    std::condition_variable _cv;
    std::mutex _mtx;

    // 线程数设置
    std::atomic_uint32_t _minThreadNum; // 最小线程数
    std::atomic_uint32_t _maxThreadNum; // 最大线程数

    // 线程池状态
                                    // 总消费者线程数 = _consumerQueue.size()
    std::atomic_uint32_t _runCnt;   // 当前工作的线程数
                                    // 当前挂起的线程数 = _consumerQueue.size() - _runCnt
    std::atomic_uint32_t _delCnt;   // 需要删除的线程数                     
    std::atomic_bool _isRun;        // 线程池是否在运行
};

int main() {
    using namespace std::chrono;
    ThreadPool tp;
    tp.run(std::chrono::milliseconds {1000}, ThreadPoolDefaultStrategy);
    tp.addTask([]{
        for (int i = 0; i < 10; ++i) {
            HX::print::println("i = ", i);
            std::this_thread::sleep_for(1s);
        }
    });
    tp.addTask([]{
        for (int j = 0; j < 5; ++j) {
            HX::print::println("j = ", j);
            std::this_thread::sleep_for(1s);
        }
    });
    std::this_thread::sleep_for(3s);
    HX::print::println("End~");
    return 0;
}