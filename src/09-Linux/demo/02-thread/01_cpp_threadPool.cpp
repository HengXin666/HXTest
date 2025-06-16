#include <HXprint/print.h>
#include <thread>
#include <functional>
#include <queue>
#include <chrono>
#include <mutex>
#include <shared_mutex>
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

    template <typename... Args>
    decltype(auto) emplace(Args&&... args) {
        std::unique_lock _{_mtx};
        return _queue.emplace(std::forward<Args>(args)...);
    }

    std::size_t size() const {
        std::shared_lock _{_mtx};
        return _queue.size();
    }
private:
    std::queue<T> _queue;
    std::shared_mutex _mtx;
};

struct ThreadPool {
    enum class Model {
        Synchronous,
        Asynchronous
    };

    ThreadPool()
        : _taskQueue{}
        , _opThread{}
        , _consumerQueue{}
        , _minThreadNum{1}
        , _maxThreadNum{std::thread::hardware_concurrency()}
        , _isRun{false}
    {}

    /**
     * @brief 添加任务, 任务应当为右值传入
     * @tparam Func 右值传入
     * @tparam Args 如果为左值, 请自行权衡生命周期, 以防止悬垂引用
     * @param func 
     * @param args 
     */
    template <typename Func, typename... Args>
    void addTask(Func&& func, Args&&... args) {
        _taskQueue.emplace([&, func = std::move(func)] {
            func(std::forward<Args>(args)...);
        });
    }

    /**
     * @brief 启动线程池
     */
    template <Model Md = Model::Asynchronous>
    void run(std::chrono::milliseconds checkTimer) {
        _isRun = true;
        if constexpr (Md == Model::Asynchronous) {
            _opThread = std::thread{&ThreadPool::check, this, checkTimer};
        } else if constexpr (Md == Model::Synchronous) {
            check(checkTimer);
        } else {
            // 内部错误
            static_assert(sizeof(Md) < 0, "Error");
        }
    }

    ThreadPool& operator=(ThreadPool const&) = delete;

    ~ThreadPool() noexcept {
        _isRun = false;
        _opThread.join();
    }
private:
    // 管理者检查线程
    void check(std::chrono::milliseconds checkTimer) {
        while (_isRun) [[likely]] {
            HX::print::println("check~");
            std::this_thread::sleep_for(checkTimer);
        }
    }

    // 任务队列
    SafeQueue<std::function<void()>> _taskQueue;

    // 管理者线程
    std::thread _opThread;

    // 消费者队列
    SafeQueue<std::unique_ptr<std::thread>> _consumerQueue;

    // 线程数设置
    std::size_t _minThreadNum; // 最小线程数
    std::size_t _maxThreadNum; // 最大线程数

    // 是否运行
    std::atomic_bool _isRun;
};

int main() {
    ThreadPool tp;
    tp.run(std::chrono::milliseconds {1000});
    return 0;
}