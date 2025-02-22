// rpc_server.cpp
#include <functional>
#include "rpc_server.h"

namespace starry {

RpcServer::RpcServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "RpcServer") {
  // 设置连接回调
  server_.setConnectionCallback(
      std::bind(&RpcServer::onConnection, this, std::placeholders::_1));
}

RpcServer::~RpcServer() {
  // 不需要手动释放 services_ 中的指针，假设他们由外部管理
}

void RpcServer::setThreadNum(int numThreads) {
  server_.setThreadNum(numThreads);
}

void RpcServer::start() {
  server_.start();
}

void RpcServer::registerService(google::protobuf::Service* service) {
  const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
  services_[desc->name()] = service;
}

void RpcServer::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    // 创建新的RpcChannel
    std::unique_ptr<RpcChannel> channel(new RpcChannel(conn));

    // 设置服务映射表
    channel->setServiceMap(services_);

    // 保存RpcChannel
    channels_[conn] = std::move(channel);
  } else {
    // 连接断开，移除对应的RpcChannel
    channels_.erase(conn);
  }
}

}  // namespace starry
