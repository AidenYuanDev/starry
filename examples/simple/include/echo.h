#pragma once

#include "buffer.h"
#include "callbacks.h"
#include "eventloop.h"
#include "inet_address.h"
#include "tcp_server.h"

class EchoServer {
public:
  EchoServer(starry::EventLoop* loop, const starry::InetAddress& listenAddr);

  void start();

private:
  void onConnection(const starry::TcpConnectionPtr& conn);
  void onMessage(const starry::TcpConnectionPtr& conn, starry::Buffer* buf, starry::Timestamp time);
  starry::TcpServer server_;
};
