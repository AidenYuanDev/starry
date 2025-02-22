#pragma once

#include <functional>
#include "buffer.h"
#include "rpc_message.pb.h"

namespace starry {

// 编码/解码错误处理回调
using RpcErrorCallback = std::function<void(const rpc::core::RpcMessage&)>;

class RpcCodec {
 public:
  explicit RpcCodec(const RpcErrorCallback& errorCb);

  // 编码消息到Buffer
  void encode(Buffer* buf, const rpc::core::RpcMessage& message);

  // 从Buffer解码消息
  void decode(Buffer* buf);

  // 设置消息回调
  using RpcMessageCallback = std::function<void(const rpc::core::RpcMessage&)>;
  void setMessageCallback(const RpcMessageCallback& cb) {
    messageCallback_ = cb;
  }

 private:
  // 解析一条完整消息
  bool parse(Buffer* buf, rpc::core::RpcMessage* message);

  RpcErrorCallback errorCallback_;
  RpcMessageCallback messageCallback_;

  // 消息头部长度(4字节)
  static const int kHeaderLen = sizeof(int32_t);
};

}  // namespace starry
