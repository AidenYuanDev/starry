#pragma once

#include <google/protobuf/service.h>
#include "echo_service.pb.h"

namespace starry {
namespace examples {

// 手动定义 EchoService 接口
class EchoService : public google::protobuf::Service {
 public:
  virtual ~EchoService() = default;

  virtual void Echo(google::protobuf::RpcController* controller,
                    const EchoRequest* request,
                    EchoResponse* response,
                    google::protobuf::Closure* done) = 0;

  // 必须实现的方法
  virtual const google::protobuf::ServiceDescriptor* GetDescriptor() {
    return descriptor();
  }

  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done) {
    if (method->name() == "Echo") {
      Echo(controller, dynamic_cast<const EchoRequest*>(request),
           dynamic_cast<EchoResponse*>(response), done);
    }
  }

  virtual const google::protobuf::Message& GetRequestPrototype(
      const google::protobuf::MethodDescriptor* method) const {
    return EchoRequest::default_instance();
  }

  virtual const google::protobuf::Message& GetResponsePrototype(
      const google::protobuf::MethodDescriptor* method) const {
    return EchoResponse::default_instance();
  }

  static const google::protobuf::ServiceDescriptor* descriptor() {
    // 这里需要实现，但临时演示可以先返回 nullptr
    return nullptr;
  }
};

// 手动定义客户端存根
class EchoService_Stub {
 public:
  explicit EchoService_Stub(google::protobuf::RpcChannel* channel)
      : channel_(channel) {}

  void Echo(google::protobuf::RpcController* controller,
            const EchoRequest* request,
            EchoResponse* response,
            google::protobuf::Closure* done) {
    channel_->CallMethod(nullptr, controller, request, response, done);
  }

 private:
  google::protobuf::RpcChannel* channel_;
};

}  // namespace examples
}  // namespace starry
