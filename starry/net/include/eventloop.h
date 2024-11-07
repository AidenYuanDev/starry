#pragma once

#include "callbacks.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace starry {

class Channel;
class Poller;
class TimerQueue;

class EventLoop {
 public:
  using Functor = std::function<void()>;
  using ChannelList = std::vector<Channel*>;

  EventLoop();
  ~EventLoop();

  // 核心功能
  void loop();  // 循环组织 Channel 和 Poller
  void quit();  // 离开循环

  // 定时器相关
  Timestamp pollReruenTime() const { return pollReturnTime_; }
  int64_t iteration() const { return iteration_; }

  // 预处理函数
  void runInLoop(Functor cb);  // 上层调用在当前EventLoop中调用
  void queueInLoop(Functor cb);  // 允许其他线程安全的向EventLoop所属的线程提交任务
  size_t queueSize();  // 预处理函数的个数

  // 内部唤醒 EventLoop
  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

  // 调试
  void assertInLoopThread() {  // 断言是否在当前线程
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
  }
  bool eventHandling() const { return eventHandling_; }

  // 获取当前的 EventLoop 实例
  static EventLoop* getEventLoopOfCurrentThread();

 private:
  void handleRead();         // 唤醒当前Loop
  void doPendingFunctors();  // 处理预处理函数

  // 调试
  void abortNotInLoopThread();  // 致命错误，不在当前线程
  void printActiceChannels() const;

  // EventLoop的状态
  std::atomic<bool> looping_;                 // 是否开启loop
  std::atomic<bool> quit_;                    // 是否离开循环
  std::atomic<bool> eventHandling_;           // 是否在处理Channel
  std::atomic<bool> callingPendingFunctors_;  // 是否在调用待处理函数
  std::thread::id threadId_;                  // 当前线程Id

  // EventLoop的所持有的Channel
  ChannelList activeChannels_;  // 监听到活跃的Channel的集和
  Channel* currentActiveChannel_;

  // EventLoop的所持有的预处理函数函数的集和
  std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;

  // 唤醒epoll_wait
  int wakeupFd_;                            // 唤醒 EventLoop 事件描述符
  std::unique_ptr<Channel> wakeupChannel_;  // 唤醒 EventLoop 事件描述符的Channel

  // 定时器
  int64_t iteration_;                       // 通过记录循环次数来预估定时时间，减少查看系统时间的次数，提高性能
  // std::unique_ptr<TimerQueue> timerQueue_;  //  在EventLoop中前向声明的对象，且只由EventLoop独占故用unique_ptr

  // Poller
  Timestamp pollReturnTime_;  // poll返回时间，用来推导定时任务的结束时间
  std::unique_ptr<Poller> poller_;                        //  在EventLoop中前向声明的对象，且只由EventLoop独占故用unique_ptr
};

}  // namespace starry
