#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstring>
#include <format>
#include <sstream>
#include <string_view>
#include <type_traits>

// 缓冲日志 + 接收日志
namespace starry {

template <typename T>
concept OstreamOutputtable =
    (!std::is_pointer_v<T> || std::is_same_v<std::decay_t<T>, char*>) &&
    requires(std::ostringstream& oss, T value) {
      { oss << value } -> std::convertible_to<std::ostream&>;
    };

// 处理除了字符串以外的指针
template <typename T>
concept AddressPrintablePointer =
    std::is_pointer_v<T> && !std::is_same_v<std::decay_t<T>, char*>;
constexpr size_t kSmallBuffer = 4 * 1024;
constexpr size_t kLargeBuffer = 4 * 1024 * 1024;

template <size_t SIZE>
class LogStream {
 public:
  LogStream() : data_(), cur_(data_.begin()) {}

  // 模板接收日志
  // 处理指针
  LogStream& operator<<(AddressPrintablePointer auto const& value) {
    if (value == nullptr) {
      append("(null)");
    } else {
      append(std::format("{}", static_cast<const void*>(value)));
    }
    return *this;
  }

  // 处理所有可以输出到流的类型（包括字符串、数字等）
  LogStream& operator<<(OstreamOutputtable auto const& value) {
    std::ostringstream oss;
    oss << value;
    append(oss.str());
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
  [[nodiscard]] int length() const {
    return static_cast<int>(cur_ - data_.begin());
  }
  [[nodiscard]] int avail() const {
    return static_cast<int>(data_.end() - cur_);
  }
  void clear() { cur_ = data_.begin(); }

 private:
  std::array<char, SIZE> data_;
  typename std::array<char, SIZE>::iterator cur_;
};

using SmallLogStream = LogStream<kSmallBuffer>;
using LargeLogStream = LogStream<kLargeBuffer>;

}  // namespace starry
