#include "eventloop.h"
#include "eventloop_thread.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include "eventloop_threadpool.h"

using namespace starry;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

// 创建线程，并开启，threads_ 持有线程的指针， loops_ 持有线程
void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  for (int i = 0; i < numThreads_; i++) {
    EventLoopThread* t = new EventLoopThread(cb);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
  }

  if (!numThreads_ && cb) {
    cb(baseLoop_);
  }
}

// 利用 next_ 获取下一个线程
EventLoop* EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoop* loop = baseLoop_;
  
  if (!loops_.empty()) {
    loop = loops_[next_++];
    next_ %= loops_.size();
  }
  return loop;
}

// 获取指定线程
EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
  baseLoop_->assertInLoopThread();
  EventLoop* loop = baseLoop_;

  if (!loops_.empty()) {
    loop = loops_[hashCode % loops_.size()];
  }
  return loop;
}

// 获取全部线程
std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  if (loops_.empty()) {
    return std::vector<EventLoop*>(1, baseLoop_);
  } else {
    return loops_;
  }
}
