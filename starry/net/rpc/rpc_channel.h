#pragma once

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/service.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "buffer.h"
#include "callbacks.h"
#include "rpc.pb.h"
#include "rpc_codec.h"
#include "service.h"
#include "types.h"

namespace google::protobuf {

class Descriptor;
class ServiceDescriptor;
class MethodDescriptor;
class Message;
using MessagePtr = ::std::shared_ptr<Message>;
}  // namespace google::protobuf

namespace starry {

class RpcController;
class Service;

class RpcChannel {
 public:
  using ServiceMap = std::map<std::string, Service*>;
  using ClientDoneCallback =
      std::function<void(const ::google::protobuf::MessagePtr&)>;

  RpcChannel();

  explicit RpcChannel(const TcpConnectionPtr& conn);

  ~RpcChannel();

  void setConnection(const TcpConnectionPtr& conn) { conn_ = conn; }

  const ServiceMap* getServices() const { return services_; }

  void setServices(const ServiceMap* services) { services_ = services; }

  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  const ::google::protobuf::Message& request,
                  const ::google::protobuf::Message* response,
                  const ClientDoneCallback& done);

  template <typename Output>
  static void downcastcall(
      const ::std::function<void(const std::shared_ptr<Output>&)>& done,
      const ::google::protobuf::MessagePtr& output) {
    done(std::shared_ptr<Output>(
        output, google::protobuf::DownCastMessage<Output>(output.get())));
  }

  template <typename Output>
  void CallMethod(
      const ::google::protobuf::MethodDescriptor* method,
      const ::google::protobuf::Message& request,
      const Output* response,
      const ::std::function<void(const std::shared_ptr<Output>&)>& done) {
    CallMethod(method, request, response,
               std::bind(&downcastcall<Output>, done, _1));
  }

  void onDisconnect();

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

 private:
  void onRpcMessage(const TcpConnectionPtr& conn,
                    const RpcMessagePtr& messagePtr,
                    Timestamp receiveTime);

  void callServiceMethod(const RpcMessage& message);
  void doneCallback(const ::google::protobuf::Message* responsePrototype,
                    const ::google::protobuf::Message* response,
                    int64_t id);

  struct OutstandingCall {
    const ::google::protobuf::Message* response;
    ClientDoneCallback done;
  };

  RpcCodec codec_;
  TcpConnectionPtr conn_;
  std::atomic<int64_t> id_;

  std::mutex mutex_;
  std::map<int64_t, OutstandingCall> outstandings_;

  const ServiceMap* services_;
};

using RpcChannelPtr = std::shared_ptr<RpcChannel>;
}  // namespace starry
