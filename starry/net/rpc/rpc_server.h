// rpc_server.h
#pragma once

#include <google/protobuf/service.h>
#include <map>
#include <memory>
#include <string>

#include "eventloop.h"
#include "inet_address.h"
#include "rpc_channel.h"
#include "tcp_server.h"

namespace starry {

class RpcServer {
 public:
  RpcServer(EventLoop* loop, const InetAddress& listenAddr);
  ~RpcServer();

  // 设置线程数
  void setThreadNum(int numThreads);

  // 启动服务器
  void start();

  // 注册服务
  void registerService(google::protobuf::Service* service);

 private:
  // 当有新连接时的回调
  void onConnection(const TcpConnectionPtr& conn);

  // TCP服务器
  TcpServer server_;

  // 服务映射表
  std::map<std::string, google::protobuf::Service*> services_;

  // 连接到RpcChannel的映射
  std::map<TcpConnectionPtr, std::unique_ptr<RpcChannel>> channels_;
};

}  // namespace starry
