#include "callbacks.h"
#include "channel.h"
#include "eventloop.h"
#include "logging.h"
#include "poller.h"
#include "sockets_ops.h"
#include "timer_id.h"
#include "timer_queue.h"

#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

using namespace starry;

thread_local EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

// 创建唤醒 looping_ 的fd
int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_FATAL << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

// 正确设置 wakeupFd_ ,使之能唤醒 looping_
EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(std::this_thread::get_id()),
      poller_(Poller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr){
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

// 关闭 wakeupFd_ 当前线程
EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << std::this_thread::get_id();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

// 从 poller_ 获得 activeChannels_, 处理活跃的 channel 和 预处理函数
void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    ++iteration_;
    if (Logger::logLevel() <= LogLevel::TRACE) {
      printActiceChannels();
    }
    eventHandling_ = true;
    for (Channel* channel : activeChannels_) {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }
    currentActiveChannel_ = nullptr;
    eventHandling_ = false;
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

// 离开循环，唤醒 looping_ 确保成功离开
void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

// 如果是当前线程就执行，否则就加入到队列中等待执行
void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

// 把 cb 放到 pendingFunctors_ 中
void EventLoop::queueInLoop(Functor cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

// 返回 pendingFunctors_ 的大小，访问时要加锁
size_t EventLoop::queueSize() {
  std::lock_guard<std::mutex> lock(mutex_);
  return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
  Timestamp time =
      std::chrono::system_clock::now() +
      std::chrono::nanoseconds(static_cast<int64_t>(delay * 1e9));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
  Timestamp time =
      std::chrono::system_clock::now() +
      std::chrono::nanoseconds(static_cast<int64_t>(interval * 1e9));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) {
  return timerQueue_->cancel(timerId);
}

// 调用 poller_ 的函数更新 channel
void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

// 调用 poller_ 的函数，移除指定 channel 或者 非活跃的 channel
void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
           std::find(activeChannels_.begin(), activeChannels_.end(), channel) ==
               activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

// 调用 poller_ 的函数，查看是否有该 channel
bool EventLoop::hasChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

// 打印崩溃信息
void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop" << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << std::this_thread::get_id();
}

// 向 wakeupFd_ 发送1个字节，唤醒 looping
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

// 接收唤醒的哪一个字节
void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

// 执行预处理函数
void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors) {
    functor();
  }
  callingPendingFunctors_ = false;
}

// 调试：打印活跃的 channel
void EventLoop::printActiceChannels() const {
  for (const Channel* channel : activeChannels_) {
    LOG_TRACE << "{" << channel->reventsToString() << "} ";
  }
}
