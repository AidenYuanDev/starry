#include "rpc_channel.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <cassert>
#include <cstdint>
#include <functional>
#include <mutex>

#include "buffer.h"
#include "callbacks.h"
#include "logging.h"
#include "rpc.pb.h"

namespace starry {

RpcChannel::RpcChannel()
    : codec_(std::bind(&RpcChannel::onRpcMessage,
                       this,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       std::placeholders::_3)),
      services_(nullptr) {
  LOG_INFO << "RpcChannel::ctor - " << this;
}

RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
    : codec_(std::bind(&RpcChannel::onRpcMessage,
                       this,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       std::placeholders::_3)),
      conn_(conn),
      services_(nullptr) {
  LOG_INFO << "RpcChannel::ctol - " << this;
}

RpcChannel::~RpcChannel() {
  LOG_INFO << "RpcChannel::dtor - " << this;
}

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            const ::google::protobuf::Message&,
                            const ::google::protobuf::Message* response,
                            const ClientDoneCallback& done) {
  RpcMessage message;
  message.set_type(REQUEST);
  int64_t id = ++id_;
  message.set_id(id);
  message.set_service(method->service()->full_name());
  message.set_method(method->name());
  message.set_request(response->SerializeAsString());

  OutstandingCall out = {response, done};
  {
    std::lock_guard<std::mutex> lock(mutex_);
    outstandings_[id] = out;
  }
  codec_.send(conn_, message);
}

void RpcChannel::onDisconnect() {}

void RpcChannel::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime) {
  LOG_TRACE << "RpcChannel::onMessage " << buf->readableBytes();
  codec_.onMessage(conn, buf, receiveTime);
}

void RpcChannel::onRpcMessage(const TcpConnectionPtr& conn,
                              const RpcMessagePtr& messagePtr,
                              Timestamp) {
  assert(conn == conn_);
  RpcMessage& message = *messagePtr;
  LOG_TRACE << "RpcChannel::onRpcMessage " << message.DebugString();
  if (message.type() == RESPONSE) {
    int64_t id = message.id();

    OutstandingCall out = {nullptr, nullptr};
    bool found = false;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::map<int64_t, OutstandingCall>::iterator it = outstandings_.find(id);
      if (it != outstandings_.end()) {
        out = it->second;
        outstandings_.erase(it);
        found = true;
      }
    }

    if (!found) {
      LOG_ERROR << "Unknown RESPONSE";
    }

    if (out.response) {
      ::google::protobuf::MessagePtr response(out.response->New());
      response->ParseFromString(message.response());
      if (out.done) {
        out.done(response);
      }
    } else {
      LOG_ERROR << "No Response prototype";
    }
  } else if (message.type() == REQUEST) {
    callServiceMethod(message);
  } else if (message.type() == ERROR) {
  }
}

void RpcChannel::callServiceMethod(const RpcMessage& message) {
  if (services_) {
    ServiceMap::const_iterator it = services_->find(message.service());
    if (it != services_->end()) {
      Service* service = it->second;
      assert(service != nullptr);
      const ::google::protobuf::ServiceDescriptor* desc =
          service->GetDescriptor();
      const ::google::protobuf::MethodDescriptor* method =
          desc->FindMethodByName(message.method());
      if (method) {
        ::google::protobuf::MessagePtr request(
            service->GetRequestPrototype(method).New());
        request->ParseFromString(message.request());
        int64_t id = message.id();
        const ::google::protobuf::Message* responsePrototype =
            &service->GetResponsePrototype(method);
        service->CallMethod(
            method, request, responsePrototype,
            std::bind(&RpcChannel::doneCallback, this, responsePrototype,
                      std::placeholders::_1, id));
      } else {
      }
    } else {
    }
  } else {
  }
}

void RpcChannel::doneCallback(
    const ::google::protobuf::Message* responsePrototype,
    const ::google::protobuf::Message* response,
    int64_t id) {
  assert(response->GetDescriptor() == responsePrototype->GetDescriptor());
  RpcMessage message;
  message.set_type(RESPONSE);
  message.set_id(id);
  message.set_response(response->SerializeAsString());
  codec_.send(conn_, message);
}

}  // namespace starry
