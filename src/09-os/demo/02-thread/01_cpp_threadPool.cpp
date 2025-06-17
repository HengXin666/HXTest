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

namespace {

struct A {
    // A& operator=(A&&) = delete;

    ~A() noexcept {
        HX::print::println("~A");
    }
};

}

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
        T res = std::move(_queue.front());
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
class Result {
    struct _Result {
        using __DataType = HX::NonVoidType<T>;

        _Result()
            : _data{}
            , _exception{}
            , _mtx{}
            , _cv{}
            , _isResed(false)
        {}

        ~_Result() noexcept {
            if (_isResed && !_exception) {
                _data.~__DataType();
            }
        }

        _Result(_Result&&) = delete;
        _Result(const _Result&) = delete;
        _Result& operator=(const _Result&) = delete;
        _Result& operator=(_Result&&) = delete;

        void wait() {
            std::unique_lock lck{_mtx};
            _cv.wait(lck, [this] { return _isResed; });
        }

        void ready() {
            {
                std::lock_guard _{_mtx};
                _isResed = true;
            }
            _cv.notify_all();
        }

        __DataType data() {
            if (_exception) [[unlikely]] {
                std::rethrow_exception(_exception);
            }
            return std::move(_data);
        }

        void setData(__DataType&& data) {
            new (std::addressof(_data)) __DataType(std::move(data));
            ready();
        }

        void unhandledException() noexcept {
            _exception = std::current_exception();
            ready();
        }
    private:
        __DataType _data;
        std::exception_ptr _exception;
        std::mutex _mtx;
        std::condition_variable _cv;
        bool _isResed;
    };
public:
    using FutureResult = _Result;

    Result()
        : _res{std::make_shared<FutureResult>()}
    {}

    Result(Result const&) = delete;
    Result(Result&&) = default;
    Result& operator=(Result const&) = delete;
    Result& operator=(Result&&) = default;

    HX::NonVoidType<T> get() {
        wait();
        return _res->data();
    }

    std::shared_ptr<FutureResult> getFutureResult() noexcept {
        return _res;
    }

    void wait() {
        _res->wait();
    }
private:
    std::shared_ptr<FutureResult> _res;
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
        _taskQueue.emplace([&, func = std::move(func), 
                            ans = res.getFutureResult()] {
            try {
                if constexpr (std::is_void_v<Res>) {
                    static_cast<void>(ans);
                    func(std::forward<Args>(args)...);
                } else {
                    ans->setData(func(std::forward<Args>(args)...));
                }
            } catch (...) {
                ans->unhandledException();
            }
            // @todo 可能的问题:
            // 如果 Result<Res> 在外部被析构, 那么内部如果还没有执行完, 那么 ans 就是
            // 悬挂引用, 这样就会出现异常...?
            // 所以 Result<Res> 内部使用 共享智能指针 就比较安全
        });
        _cv.notify_one();
        return res;
    }

    /**
     * @brief 启动线程池
     * @tparam Md 运行模式 (默认为新建一个管理者线程 (异步))
     * @tparam Strategy 评判是否增删线程的方法 (返回值为 int(表示增删的线程数量), 传参为`ThreadPoolData const&`)
     * @param checkTimer 管理者轮询线程的时间间隔
     * @param strategy 是否增删线程的回调函数
     */
    template <Model Md = Model::Asynchronous, typename Strategy,
        typename = std::enable_if_t<
            std::is_same_v<decltype(std::declval<Strategy>()(std::declval<ThreadPoolData const&>())), int>>>
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
            auto workerSize = static_cast<uint32_t>(_workers.size());
            if (int add = strategy(ThreadPoolData{
                _minThreadNum,
                _maxThreadNum,
                taskSize,
                workerSize,
                runCnt,
                workerSize - runCnt
            })) {
                if (add > 0) {
                    makeWorker(static_cast<std::size_t>(add));
                } else {
                    add = -add;
                    _delCnt = static_cast<uint32_t>(add);
                    for (int i = 0; i < add; ++i) {
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
                std::function<void()> task;
                while (_isRun) [[likely]] {
                    {
                        // 挂起, 并等待任务
                        std::unique_lock lck{_mtx};
                        _cv.wait(lck, [&] {
                            return !_taskQueue.empty() || _delCnt > 0 || !_isRun;
                        });
                        if (!_isRun) [[unlikely]] {
                            break;
                        }
                        if (tryDecrementIfPositive(_delCnt)) {
                            _delIdQueue.emplace(std::this_thread::get_id());
                            break;
                        }
                        task = _taskQueue.frontAndPop();
                    }
                    ++_runCnt;
                    task(); // 执行任务
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
            if (it == _workers.end()) [[unlikely]] {
                throw std::runtime_error{"iterator failure"};
            }
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
    auto res = tp.addTask([]{
        for (int i = 0; i < 10; ++i) {
            HX::print::println("i = ", i);
            std::this_thread::sleep_for(100ms);
        }
        throw std::runtime_error{"test Error!"};
        return 114514;
    });
    {
        tp.addTask([]{
            for (int i = 0; i < 10; ++i) {
                HX::print::println("j = ", i);
                std::this_thread::sleep_for(100ms);
            }
            return A{};
        });
    }
    try {
        HX::print::println("End~ ", res.get());
    } catch (std::exception& e) {
        HX::print::println("Error: ", e.what());
    }
    return 0;
}