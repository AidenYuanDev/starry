#pragma once

#include <map>
#include <string>
#include "callbacks.h"
#include "eventloop.h"
#include "inet_address.h"
#include "rpc_service.h"
#include "tcp_server.h"

namespace starry {

class Service;

class RpcServer {
 public:
  RpcServer(EventLoop* loop, const InetAddress& listnAddr);

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void registerService(Service*);
  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  TcpServer server_;
  std::map<std::string, Service*> services_;
  RpcServiceImpl metaService_;
};

}  // namespace starry
