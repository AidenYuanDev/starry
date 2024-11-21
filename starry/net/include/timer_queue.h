#pragma once

#include <cstdint>
#include <set>
#include <utility>
#include <vector>

#include "callbacks.h"
#include "channel.h"

namespace starry {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  // 添加新的定时器
  TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
  // 取消一个定时器
  void cancel(TimerId timerId);

 private:
  using Entry = std::pair<Timestamp, Timer*>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(Timer* timer);   // 添加定时器回调函数
  void cancelInLoop(TimerId timerId);  // 取消定时器回调函数
  void handleRead();                   // 处理 timerfd 的读时间，即定时器触发
  std::vector<Entry> getExpired(Timestamp now);  // 获取所有以过期的定时器
  void reset(const std::vector<Entry>& expired,
             Timestamp now);  // 重置定时器状态

  bool insert(Timer* timer);  // 插入定时器

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  // O(1)时间找到要触发的定时器activeTimers_
  TimerList timers_;
  // 通过定时器指针和序号，快速查找到对应定时器，用来取消定时
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_;
  ActiveTimerSet cancelingTimers_; // 取消的定时器
};

}  // namespace starry
