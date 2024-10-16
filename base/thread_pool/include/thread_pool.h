#include <atomic>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace starry {

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads, size_t maxQueueSize = 0)
      : maxQueueSize_(maxQueueSize),
        stop_(false),
        running_(false),
        threadCount_(threads) {
    start();
  }

  ~ThreadPool() {
    if (running_)
      stop();
  }

  void start() {
    std::unique_lock lock(queue_mutex_);
    if (running_)
      return;
    running_ = true;
    stop_ = false;

    workers_.reserve(threadCount_);
    for (size_t i = 0; i < threadCount_; ++i) {
      workers_.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty())
              return;
            task = std::move(tasks_.front());
            tasks_.pop();
          }
          notFull_.notify_one();
          task();
        }
      });
    }
  }

  void stop() {
    {
      std::unique_lock lock(queue_mutex_);
      if (!running_)
        return;
      stop_ = true;
      running_ = false;
    }
    condition_.notify_all();
    for (auto& worker : workers_) {
      worker.join();
    }
    workers_.clear();
  }

  template <typename F, typename... Args>
    requires std::invocable<F, Args...>
  auto enqueue(F&& f, Args&&... args) {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock lock(queue_mutex_);
      if (!running_) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
      }

      if (maxQueueSize_ > 0) {
        notFull_.wait(lock, [this] { return tasks_.size() < maxQueueSize_; });
      }

      tasks_.emplace([task] { (*task)(); });
    }
    condition_.notify_one();
    return res;
  }

  void setMaxQueueSize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    maxQueueSize_ = maxSize;
  }

  size_t size() const { return threadCount_; }

  size_t queueSize() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
  }

  bool isRunning() const { return running_; }

 private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  mutable std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::condition_variable notFull_;
  size_t maxQueueSize_;
  std::atomic<bool> stop_;
  std::atomic<bool> running_;
  size_t threadCount_;
};

}
