// rpc_channel.h
#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <google/protobuf/service.h>
#include "buffer.h"
#include "codec.h"
#include "rpc_closure.h"
#include "rpc_message.pb.h"
#include "tcp_connection.h"

namespace starry {

// 前向声明
class RpcController;

class RpcChannel : public google::protobuf::RpcChannel {
 public:
  // 构造函数，需要一个TCP连接
  explicit RpcChannel(const TcpConnectionPtr& conn);

  // 实现RpcChannel接口
  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done) override;

  // 用于服务器端的回调设置
  using ServiceMap = std::map<std::string, google::protobuf::Service*>;
  void setServiceMap(const ServiceMap& services) { services_ = services; }

  // 关闭通道
  void close();

 private:
  // 处理RPC响应
  void onRpcMessage(const rpc::core::RpcMessage& message);

  // 处理连接关闭
  void onConnectionClosed();

  // 处理错误
  void onRpcError(const rpc::core::RpcMessage& errorMsg);

  // 在服务器端处理请求
  void handleRequest(const rpc::core::RpcMessage& request);

  // 请求结构
  struct OutstandingCall {
    google::protobuf::Message* response;
    google::protobuf::Closure* done;
    google::protobuf::RpcController* controller;
  };

  TcpConnectionPtr conn_;
  RpcCodec codec_;
  std::atomic<uint64_t> nextId_{1};  // 从1开始，0作为无效ID
  std::map<uint64_t, OutstandingCall> outstandingCalls_;
  ServiceMap services_;  // 服务端使用，客户端为空
};

}  // namespace starry
