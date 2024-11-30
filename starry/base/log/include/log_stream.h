#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <format>
#include <sstream>
#include <string_view>
#include <type_traits>

// 缓冲日志 + 接收日志
namespace starry {

constexpr size_t kSmallBuffer = 4 * 1024;
constexpr size_t kLargeBuffer = 4 * 1024 * 1024;

template <size_t SIZE>
class LogStream {
 public:
  LogStream() : data_(), cur_(data_.begin()) {}

  // 使用单个泛型操作符处理所有输出情况
  LogStream& operator<<(const auto& value) {
    using TypeNoCV = std::remove_cvref_t<std::remove_pointer_t<decltype(value)>>;
    // 首先判断是否是指针类型
    if constexpr (std::is_pointer_v<decltype(value)>) {
      // 如果是字符指针（char* 或 const char*），我们输出字符串内容
      if constexpr (std::is_same_v<TypeNoCV, char>) {
        if (value) {
          append(std::string_view(value, strlen(value)));
        } else {
          append("(null)");
        }
      } else {  // 对于其他类型的指针，我们输出其地址
        if (value) {
          append(std::format("{}", static_cast<const void*>(value)));
        } else {
          append("(null)");
        }
      }
    } else {  // 对于非指针类型，直接使用流进行输出
      std::ostringstream oss;
      oss << value;
      append(oss.str());
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
