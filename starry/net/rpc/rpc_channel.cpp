// rpc_channel.cpp
#include <google/protobuf/descriptor.h>
#include <functional>
#include <string>
#include "rpc_channel.h"

namespace starry {

RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
    : conn_(conn),
      codec_([this](const rpc::core::RpcMessage& msg) { onRpcError(msg); }) {
  // 设置消息回调
  codec_.setMessageCallback(
      [this](const rpc::core::RpcMessage& msg) { onRpcMessage(msg); });

  if (conn_) {
    // 设置TCP连接的消息回调
    conn_->setMessageCallback([this](const TcpConnectionPtr&, Buffer* buf,
                                     Timestamp) { codec_.decode(buf); });

    // 设置关闭回调
    conn_->setCloseCallback(
        [this](const TcpConnectionPtr&) { onConnectionClosed(); });
  }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
  // 创建RPC请求消息
  rpc::core::RpcMessage message;
  message.set_type(rpc::core::RpcMessage::REQUEST);
  message.set_id(nextId_++);
  message.set_service(method->service()->name());
  message.set_method(method->name());

  // 序列化请求
  std::string requestData;
  if (!request->SerializeToString(&requestData)) {
    // 序列化失败
    if (controller) {
      controller->SetFailed("Failed to serialize request");
    }
    if (done) {
      done->Run();
    }
    return;
  }

  message.set_payload(requestData);

  // 保存调用信息，用于处理响应
  OutstandingCall call;
  call.response = response;
  call.done = done;
  call.controller = controller;
  outstandingCalls_[message.id()] = call;

  // 发送请求
  if (conn_ && conn_->connected()) {
    Buffer buf;
    codec_.encode(&buf, message);
    conn_->send(&buf);
  } else {
    // 连接已关闭
    if (controller) {
      controller->SetFailed("Connection closed");
    }
    if (done) {
      done->Run();
    }
    outstandingCalls_.erase(message.id());
  }
}

void RpcChannel::onRpcMessage(const rpc::core::RpcMessage& message) {
  switch (message.type()) {
    case rpc::core::RpcMessage::REQUEST:
      handleRequest(message);
      break;
    case rpc::core::RpcMessage::RESPONSE: {
      // 查找对应的调用
      auto it = outstandingCalls_.find(message.id());
      if (it != outstandingCalls_.end()) {
        // 反序列化响应
        if (it->second.response && !message.payload().empty()) {
          it->second.response->ParseFromString(message.payload());
        }

        // 调用回调
        if (it->second.done) {
          it->second.done->Run();
        }

        // 移除调用记录
        outstandingCalls_.erase(it);
      }
    } break;
    case rpc::core::RpcMessage::ERROR: {
      // 处理错误消息
      auto it = outstandingCalls_.find(message.id());
      if (it != outstandingCalls_.end()) {
        if (it->second.controller) {
          it->second.controller->SetFailed(message.error());
        }

        if (it->second.done) {
          it->second.done->Run();
        }

        outstandingCalls_.erase(it);
      }
    } break;
    default:
      // 未知类型
      break;
  }
}

void RpcChannel::handleRequest(const rpc::core::RpcMessage& request) {
  // 只有服务端才处理请求
  if (services_.empty()) {
    // 客户端收到请求，这是错误的
    rpc::core::RpcMessage errorMsg;
    errorMsg.set_type(rpc::core::RpcMessage::ERROR);
    errorMsg.set_id(request.id());
    errorMsg.set_error("Not a server");

    Buffer buf;
    codec_.encode(&buf, errorMsg);
    conn_->send(&buf);
    return;
  }

  // 查找服务
  auto serviceIt = services_.find(request.service());
  if (serviceIt == services_.end()) {
    // 服务不存在
    rpc::core::RpcMessage errorMsg;
    errorMsg.set_type(rpc::core::RpcMessage::ERROR);
    errorMsg.set_id(request.id());
    errorMsg.set_error("Service not found: " + request.service());

    Buffer buf;
    codec_.encode(&buf, errorMsg);
    conn_->send(&buf);
    return;
  }

  // 获取方法描述符
  auto service = serviceIt->second;
  const google::protobuf::ServiceDescriptor* serviceDesc =
      service->GetDescriptor();
  const google::protobuf::MethodDescriptor* methodDesc =
      serviceDesc->FindMethodByName(request.method());

  if (!methodDesc) {
    // 方法不存在
    rpc::core::RpcMessage errorMsg;
    errorMsg.set_type(rpc::core::RpcMessage::ERROR);
    errorMsg.set_id(request.id());
    errorMsg.set_error("Method not found: " + request.method());

    Buffer buf;
    codec_.encode(&buf, errorMsg);
    conn_->send(&buf);
    return;
  }

  // 创建请求和响应对象
  google::protobuf::Message* requestMsg =
      service->GetRequestPrototype(methodDesc).New();
  google::protobuf::Message* responseMsg =
      service->GetResponsePrototype(methodDesc).New();

  // 解析请求
  if (!requestMsg->ParseFromString(request.payload())) {
    // 请求解析失败
    rpc::core::RpcMessage errorMsg;
    errorMsg.set_type(rpc::core::RpcMessage::ERROR);
    errorMsg.set_id(request.id());
    errorMsg.set_error("Failed to parse request");

    Buffer buf;
    codec_.encode(&buf, errorMsg);
    conn_->send(&buf);

    delete requestMsg;
    delete responseMsg;
    return;
  }

  // 创建一个完成回调，负责发送响应
  uint64_t request_id = request.id();  // 保存请求ID的副本
  google::protobuf::Closure* done =
      NewCallback([this, responseMsg, request_id]() {
        // 创建响应消息
        rpc::core::RpcMessage response;
        response.set_type(rpc::core::RpcMessage::RESPONSE);
        response.set_id(request_id);

        // 序列化响应
        std::string responseData;
        if (responseMsg->SerializeToString(&responseData)) {
          response.set_payload(responseData);
        }

        // 发送响应
        Buffer buf;
        codec_.encode(&buf, response);
        if (conn_ && conn_->connected()) {
          conn_->send(&buf);
        }

        // 清理资源
        delete responseMsg;
      });

  // 调用服务方法
  service->CallMethod(methodDesc, nullptr, requestMsg, responseMsg, done);

  // 清理请求
  delete requestMsg;
}

void RpcChannel::onConnectionClosed() {
  // 连接关闭，取消所有未完成的调用
  for (auto& call : outstandingCalls_) {
    if (call.second.controller) {
      call.second.controller->SetFailed("Connection closed");
    }

    if (call.second.done) {
      call.second.done->Run();
    }
  }

  outstandingCalls_.clear();
}

void RpcChannel::onRpcError(const rpc::core::RpcMessage& errorMsg) {
  // 处理RPC错误
  uint64_t id = errorMsg.id();
  if (id != 0) {  // Proto3 中检查默认值的方式
    auto it = outstandingCalls_.find(id);
    if (it != outstandingCalls_.end()) {
      if (it->second.controller) {
        it->second.controller->SetFailed(errorMsg.error());
      }

      if (it->second.done) {
        it->second.done->Run();
      }

      outstandingCalls_.erase(it);
    }
  }
}

void RpcChannel::close() {
  if (conn_) {
    conn_->shutdown();
  }
}

}  // namespace starry
