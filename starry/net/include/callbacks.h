#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include "types.h"
namespace starry {

template<typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_case(const ::std::shared_ptr<From> &f) {
  if (false) {
    implicit_cast<From*, To*>(0);
  }
  assert(f == nullptr || dynamic_cast<To*>(f.get()) != nullptr);
  return ::std::static_pointer_cast<To>(f);
}

class Buffer;
class TcpConnection;
using Timestamp = std::chrono::system_clock::time_point;
using Clock = std::chrono::system_clock;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp receiveTime);

}  // namespace starry
