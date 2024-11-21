#pragma once

#include <cstdint>
namespace starry {

class Timer;

class TimerId {
 public:
  TimerId() : timer_(nullptr), sequence_(0) {}
  TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}

  friend class TimerQueue;

 private:
  Timer* timer_;      // 指向实际的Timer 对象
  int64_t sequence_;  // 序列号，用于区分同一个 Timer 对象的不同实例
};

}  // namespace starry
