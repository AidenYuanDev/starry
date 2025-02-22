// rpc_closure.h
#pragma once

#include <google/protobuf/service.h>
#include <functional>

namespace starry {

// RpcClosure 类：将 std::function 包装为 google::protobuf::Closure
class RpcClosure : public google::protobuf::Closure {
 public:
  // 构造函数接收一个回调函数
  explicit RpcClosure(std::function<void()> callback)
      : callback_(std::move(callback)) {}

  // 实现 Closure 接口的 Run 方法
  void Run() override {
    callback_();  // 执行回调
    delete this;  // 自我删除，避免内存泄漏
  }

 private:
  std::function<void()> callback_;  // 存储回调函数
};

// 创建 Closure 的辅助函数，简化使用
inline google::protobuf::Closure* NewCallback(std::function<void()> callback) {
  return new RpcClosure(std::move(callback));
}

}  // namespace starry
