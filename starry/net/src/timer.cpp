#include "timer.h"
#include "callbacks.h"
#include <atomic>
#include <chrono>
#include <cstdint>

using namespace starry;

std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(Timestamp now) {
  if (repeat_) {
    expiration_ = now + std::chrono::microseconds(interval_);
  } else {
    expiration_ = Timestamp::min();
  }
}
