#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <functional>
#include <map>
#include <memory>
#include "rpc_channel.h"

namespace google::protobuf {

class Descriptor;
class ServiceDescriptor;
class MethodDescriptor;
class Message;
using MessagePtr = ::std::shared_ptr<Message>;
using ConstMessagePtr = ::std::shared_ptr<const Message>;
}  // namespace google::protobuf

namespace starry {

class RpcChannel;
class Service;
class RpcController;
using RpcDoneCallback =
    ::std::function<void(const ::google::protobuf::Message*)>;
using ServiceMap = ::std::map<std::string, Service*>;

class Service {
 public:
  Service() {}
  virtual ~Service() {}

  using RpcDoneCallback = ::starry::RpcDoneCallback;
  virtual const ::google::protobuf::ServiceDescriptor* GetDescriptor() = 0;
  virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                          const ::google::protobuf::MessagePtr& request,
                          const ::google::protobuf::Message* response,
                          const RpcDoneCallback& done) = 0;

  virtual const ::google::protobuf::Message& GetRequestPrototype(
      const ::google::protobuf::MethodDescriptor* method) const = 0;
  virtual const ::google::protobuf::Message& GetResponsePrototype(
      const ::google::protobuf::MethodDescriptor* method) const = 0;
};

}  // namespace starry
