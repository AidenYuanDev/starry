#include <google/protobuf/descriptor.h>
#include <functional>
#include "callbacks.h"
#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"
#include "rpc_channel.h"
#include "rpc_server.h"
#include "service.h"

using namespace starry;

RpcServer::RpcServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "RpcServer"),
      services_(),
      metaService_(&services_) {
  server_.setConnectionCallback(std::bind(&RpcServer::onConnection, this, _1));
  registerService(&metaService_);
}

void RpcServer::registerService(Service* service) {
  const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
  services_[std::string(desc->full_name())] = service;
}

void RpcServer::start() {
  server_.start();
}

void RpcServer::onConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "RpcServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    RpcChannelPtr channel(new RpcChannel(conn));
    channel->setServices(&services_);
    conn->setMessageCallback(
        std::bind(&RpcChannel::onMessage, channel.get(), _1, _2, _3));
    conn->setContext(channel);
  } else {
    conn->setContext(RpcChannelPtr());
  }
}
