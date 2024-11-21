#pragma once

#include <atomic>
#include <cstdint>
#include <utility>
#include "callbacks.h"

namespace starry {

class Timer {
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(static_cast<int64_t>(interval * 1000)),
        repeat_(interval > 0),
        sequence_(++s_numCreated_) {}

  void run() const { callback_(); }

  Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }
  void restart(Timestamp now);
  static int64_t s_numCreated() { return s_numCreated_; }

  bool operator<(const Timer t) const {
    return expiration_ < t.expiration_;
  } 

 private:
  const TimerCallback callback_;  // 定时器回调函数
  Timestamp expiration_;          // 定时器下次触发的时间
  const int64_t interval_;  // 如果是重复定时器，这是两次触发之间的时间间隔
  const bool repeat_;       // 是否是重复定时器
  const int64_t sequence_;

  static std::atomic<int64_t> s_numCreated_;
};

}  // namespace starry
