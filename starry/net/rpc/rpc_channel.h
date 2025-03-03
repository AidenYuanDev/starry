#pragma once

#include <google/protobuf/service.h>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "callbacks.h"
#include "rpc_codec.h"

namespace starry {

class Buffer;

// RPC通道实现，用于客户端发起RPC调用和服务端处理RPC请求
class RpcChannel : public google::protobuf::RpcChannel {
 public:
  // 服务映射表类型定义
  using ServiceMap = std::map<std::string, google::protobuf::Service*>;
  // RPC完成回调函数类型定义
  using ClientDoneCallback = std::function<void(const MessagePtr&)>;

  // 构造函数
  RpcChannel();
  // 使用TCP连接构造
  explicit RpcChannel(const TcpConnectionPtr& conn);
  // 析构函数
  ~RpcChannel() override;

  // 设置TCP连接
  void setConnection(const TcpConnectionPtr& conn) { conn_ = conn; }

  // 获取服务映射表
  const ServiceMap* getServices() const { return services_; }

  // 设置服务映射表
  void setServices(const ServiceMap* services) { services_ = services; }

  // 实现Protocol Buffers RPC接口
  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done) override;

  // 扩展的RPC调用接口，提供更现代的API
  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  const google::protobuf::Message& request,
                  const google::protobuf::Message* response,
                  const ClientDoneCallback& done);

  // 类型安全的转换辅助函数
  template <typename Output>
  static void downcastCall(
      const std::function<void(const std::shared_ptr<Output>&)>& done,
      const MessagePtr& output) {
    done(std::dynamic_pointer_cast<Output>(output));
  }

  // 类型安全的RPC调用方法
  template <typename Output>
  void CallMethod(
      const google::protobuf::MethodDescriptor* method,
      const google::protobuf::Message& request,
      const Output* response,
      const std::function<void(const std::shared_ptr<Output>&)>& done) {
    CallMethod(method, request, response,
               std::bind(&RpcChannel::downcastCall<Output>, done,
                         std::placeholders::_1));
  }

  // 处理连接断开
  void onDisconnect();

  // 处理收到的消息
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

 private:
  // 处理RPC消息回调
  void onRpcMessage(const TcpConnectionPtr& conn,
                    const RpcMessagePtr& messagePtr,
                    Timestamp receiveTime);

  // 调用本地服务方法
  void callServiceMethod(const RpcMessage& message);

  // 服务方法完成回调
  void doneCallback(const google::protobuf::Message* responsePrototype,
                    const google::protobuf::Message* response,
                    int64_t id);

  // 自定义Closure类，用于处理回调
  class DoneCallbackClosure : public google::protobuf::Closure {
   public:
    DoneCallbackClosure(RpcChannel* channel,
                        const google::protobuf::Message* responsePrototype,
                        const google::protobuf::Message* response,
                        int64_t id)
        : channel_(channel),
          responsePrototype_(responsePrototype),
          response_(response),
          id_(id) {}

    void Run() override {
      channel_->doneCallback(responsePrototype_, response_, id_);
      delete this;  // 自删除
    }

   private:
    RpcChannel* channel_;
    const google::protobuf::Message* responsePrototype_;
    const google::protobuf::Message* response_;
    int64_t id_;
  };

  // 未完成调用结构
  struct OutstandingCall {
    // 使用std::shared_ptr替代裸指针，提高安全性
    std::shared_ptr<google::protobuf::Message> response;
    ClientDoneCallback done;
  };

  RpcCodec codec_;              // RPC编解码器
  TcpConnectionPtr conn_;       // TCP连接
  std::atomic<int64_t> id_{0};  // 请求ID计数器

  std::mutex mutex_;  // 保护outstandings_的互斥锁
  std::map<int64_t, OutstandingCall> outstandings_;  // 未完成调用映射表

  const ServiceMap* services_{nullptr};  // 服务映射表
};

// RpcChannel智能指针类型
using RpcChannelPtr = std::shared_ptr<RpcChannel>;

}  // namespace starry
