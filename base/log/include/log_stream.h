#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <string_view>

#include "noncopyable.h"

// 缓冲日志 + 接收日志
namespace starry {

constexpr size_t kSmallBuffer = 4 * 1024;
constexpr size_t kLargeBuffer = 4 * 1024 * 1024;

template <size_t SIZE>
class LogStream : noncopyable {
 public:
  LogStream() : data_(), cur_(data_.begin()) {}

  // 模板接收日志
  template <typename T>
  LogStream& operator<<(const T& value) {
    if constexpr (std::convertible_to<T, std::string_view>) {
      append(std::string_view(value));
    } else {
      append(std::to_string(value));
    }
    return *this;
  }

  void append(std::string_view str) {
    size_t len = std::min(str.size(), static_cast<size_t>(avail()));
    std::copy_n(str.begin(), len, cur_);
    cur_ += len;
  }

  [[nodiscard]] std::string_view data() const {
    return std::string_view(data_.data(), length());
  }
  [[nodiscard]] int length() const { return static_cast<int>(cur_ - data_.begin()); }
  [[nodiscard]] int avail() const { return static_cast<int>(data_.end() - cur_); }
  void clear() { cur_ = data_.begin(); }

 private:
  std::array<char, SIZE> data_;
  typename std::array<char, SIZE>::iterator cur_;
};

using SmallLogStream = LogStream<kSmallBuffer>;
using LargeLogStream = LogStream<kLargeBuffer>;

}  // namespace starry
