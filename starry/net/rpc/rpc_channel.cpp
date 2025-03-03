#include "rpc_channel.h"

#include <google/protobuf/descriptor.h>
#include <cassert>

#include "buffer.h"
#include "logging.h"
#include "rpc.pb.h"

namespace starry {

// 构造函数
RpcChannel::RpcChannel()
    : codec_(std::bind(&RpcChannel::onRpcMessage,
                       this,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       std::placeholders::_3)) {
  LOG_INFO << "RpcChannel::ctor - " << this;
}

// 带连接的构造函数
RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
    : codec_(std::bind(&RpcChannel::onRpcMessage,
                       this,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       std::placeholders::_3)),
      conn_(conn) {
  LOG_INFO << "RpcChannel::ctor - " << this;
}

// 析构函数
RpcChannel::~RpcChannel() {
  LOG_INFO << "RpcChannel::dtor - " << this;
}

// 实现Google Protocol Buffers的RpcChannel接口
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController*,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
  // 创建RPC请求消息
  RpcMessage message;
  message.set_type(REQUEST);
  int64_t id = id_.fetch_add(1, std::memory_order_relaxed);
  message.set_id(id);
  message.set_service(method->service()->full_name());
  message.set_method(method->name());
  message.set_request(request->SerializeAsString());

  // 创建ClientDoneCallback，在收到响应时调用原始的Closure
  ClientDoneCallback callback = [response,
                                 done](const MessagePtr& responseMsg) {
    if (responseMsg) {
      // 解析响应到用户提供的response对象
      response->CopyFrom(*responseMsg);
    }
    // 调用用户提供的完成回调
    if (done) {
      done->Run();
    }
  };

  // 创建并保存未完成调用
  OutstandingCall out;
  out.response = std::shared_ptr<google::protobuf::Message>(response->New());
  out.done = std::move(callback);

  {
    std::lock_guard<std::mutex> lock(mutex_);
    outstandings_[id] = std::move(out);
  }

  // 发送请求
  codec_.send(conn_, message);
}

// 扩展的RPC调用接口
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            const google::protobuf::Message& request,
                            const google::protobuf::Message* response,
                            const ClientDoneCallback& done) {
  // 创建RPC请求消息
  RpcMessage message;
  message.set_type(REQUEST);
  int64_t id = id_.fetch_add(1, std::memory_order_relaxed);
  message.set_id(id);
  message.set_service(method->service()->full_name());
  message.set_method(method->name());
  message.set_request(request.SerializeAsString());

  // 创建并保存未完成调用
  OutstandingCall out;
  out.response = std::shared_ptr<google::protobuf::Message>(response->New());
  out.done = done;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    outstandings_[id] = std::move(out);
  }

  // 发送请求
  codec_.send(conn_, message);
}

// 处理连接断开
void RpcChannel::onDisconnect() {
  // 获取并清空所有未完成调用
  std::map<int64_t, OutstandingCall> outstandings;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    outstandings.swap(outstandings_);
  }

  // 处理所有未完成调用，通知调用失败
  for (auto& pair : outstandings) {
    LOG_WARN << "RpcChannel::onDisconnect - call id=" << pair.first
             << " disconnected";
    if (pair.second.done) {
      pair.second.done(nullptr);
    }
  }
}

// 处理收到的消息
void RpcChannel::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime) {
  LOG_TRACE << "RpcChannel::onMessage " << buf->readableBytes();
  codec_.onMessage(conn, buf, receiveTime);
}

// 处理RPC消息回调
void RpcChannel::onRpcMessage(const TcpConnectionPtr& conn,
                              const RpcMessagePtr& messagePtr,
                              Timestamp) {
  assert(conn == conn_);
  RpcMessage& message = *messagePtr;

  if (message.type() == RESPONSE) {
    // 处理RPC响应
    int64_t id = message.id();

    // 查找对应的未完成调用
    OutstandingCall out;
    bool found = false;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = outstandings_.find(id);
      if (it != outstandings_.end()) {
        out = std::move(it->second);
        outstandings_.erase(it);
        found = true;
      }
    }

    if (found) {
      // 处理响应
      if (out.response && out.done) {
        // 反序列化响应
        out.response->ParseFromString(message.response());
        // 调用回调
        out.done(out.response);
      }
    } else {
      LOG_ERROR << "Unknown RPC response id: " << id;
    }
  } else if (message.type() == REQUEST) {
    // 处理RPC请求
    callServiceMethod(message);
  } else if (message.type() == ERROR) {
    // 处理RPC错误
    LOG_ERROR << "RPC error response, id: " << message.id()
              << ", error code: " << message.error();

    // 查找对应的未完成调用
    int64_t id = message.id();
    OutstandingCall out;
    bool found = false;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = outstandings_.find(id);
      if (it != outstandings_.end()) {
        out = std::move(it->second);
        outstandings_.erase(it);
        found = true;
      }
    }

    if (found && out.done) {
      // 通知调用失败
      out.done(nullptr);
    }
  }
}

// 调用本地服务方法
void RpcChannel::callServiceMethod(const RpcMessage& message) {
  // 检查是否有服务注册
  if (!services_) {
    LOG_ERROR << "RpcChannel has no service registry";
    return;
  }

  // 查找服务
  auto it = services_->find(message.service());
  if (it != services_->end()) {
    google::protobuf::Service* service = it->second;
    const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();

    // 查找方法
    const google::protobuf::MethodDescriptor* method =
        desc->FindMethodByName(message.method());

    if (method) {
      // 创建请求和响应对象
      std::unique_ptr<google::protobuf::Message> request(
          service->GetRequestPrototype(method).New());

      if (request->ParseFromString(message.request())) {
        // 响应原型
        const google::protobuf::Message* responsePrototype =
            &(service->GetResponsePrototype(method));

        // 创建响应
        std::unique_ptr<google::protobuf::Message> response(
            responsePrototype->New());

        // 调用服务方法，使用DoneCallbackClosure处理回调
        service->CallMethod(
            method, nullptr, request.get(), response.get(),
            new DoneCallbackClosure(this, responsePrototype, response.release(),
                                    message.id()));
      } else {
        LOG_ERROR << "Failed to parse request: " << message.service()
                  << "::" << message.method();
      }
    } else {
      LOG_ERROR << "Method not found: " << message.service()
                << "::" << message.method();
    }
  } else {
    LOG_ERROR << "Service not found: " << message.service();
  }
}

// 服务方法完成回调
void RpcChannel::doneCallback(
    const google::protobuf::Message* responsePrototype,
    const google::protobuf::Message* response,
    int64_t id) {
  // 断言响应类型正确
  assert(response->GetDescriptor() == responsePrototype->GetDescriptor());

  // 创建RPC响应消息
  RpcMessage message;
  message.set_type(RESPONSE);
  message.set_id(id);
  message.set_response(response->SerializeAsString());

  // 发送响应
  codec_.send(conn_, message);

  // 释放资源
  delete response;
}

}  // namespace starry
