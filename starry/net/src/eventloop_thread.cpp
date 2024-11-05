#include <eventloop.h>
#include <eventloop_thread.h>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace starry;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_(nullptr), name_(name), exiting_(false), callback_(cb) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_) {
    loop_->quit();
    if (thread_.joinable()) {
      thread_.join();
    }
  }
}

// 开启当前线程，并返回所持有的 loop_
EventLoop* EventLoopThread::startLoop() {
  thread_ = std::thread([this] { threadFunc(); });

  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] { return loop_ || exiting_; });

  return loop_;
}

// 创建一个 loop_, 并运行
void EventLoopThread::threadFunc() {
  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = &loop;
  }

  cond_.notify_one();

  loop.loop();

  std::lock_guard<std::mutex> lock(mutex_);
  loop_ = nullptr;
}
