#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>

#include "buffer.h"
#include "callbacks.h"
#include "rpc_codec.h"

namespace google::protobuf {

class Descriptor;
class ServiceDescriptor;
class MethodDescrptor;
class Message;
class Closure;
class RpcController;
class Service;

}  // namespace google::protobuf

namespace starry {
class RpcChannel : public ::google::protobuf::RpcChannel {
 public:
  RpcChannel();
  explicit RpcChannel(const TcpConnectionPtr& conn);
  ~RpcChannel() override;

  void setConnection(const TcpConnectionPtr& conn) {
    conn_ = conn;
  }

  void setServices(const std::map<std::string, ::google::protobuf::Service*>* services) {
    services_ = services;
  }

  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  ::google::protobuf::RpcController* controller,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done) override;

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp reveiveTime);

 private:
  void onRpcMessage(const TcpConnectionPtr& conn,
                    Buffer* buf,
                    Timestamp reveiveTime);

  void doneCallback(::google::protobuf::Message* response, int64_t id);

  struct OutstandingCall {
    ::google::protobuf::Message* response;
    ::google::protobuf::Closure* done;
  };

  RpcCodec codec_;
  TcpConnectionPtr conn_;
  std::atomic<int64_t> id_;

  std::mutex mutex_;
  std::map<int64_t, OutstandingCall> outstandings_;

  const std::map<std::string, ::google::protobuf::Service*>* services_;
};
typedef std::shared_ptr<RpcChannel> RpcChannelPtr;
}  // namespace starry
