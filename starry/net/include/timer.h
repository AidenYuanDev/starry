#pragma once

#include "callbacks.h"
#include <atomic>
#include <cstdint>
#include <utility>

namespace starry {

class Timer {
public:
  Timer(TimerCallback cb, Timestamp when, double interval) 
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(static_cast<int64_t>(interval * 1000)),
      repeat_(interval > 0),
      sequence_(++s_numCreated_) {}

  void run() const {
    callback_();
  }

  Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }
  void restart(Timestamp now);
  static int64_t s_numCreated() { return s_numCreated_; }

private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const int64_t interval_;
  const bool repeat_;
  const int64_t sequence_;

  static std::atomic<int64_t> s_numCreated_;
};

}
