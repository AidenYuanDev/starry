#include <format>
#include <functional>
#include "callbacks.h"
#include "echo.h"
#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"

using namespace std::placeholders;

EchoServer::EchoServer(starry::EventLoop* loop,
                       const starry::InetAddress& listenAddr)
    : server_(loop, listenAddr, "EchoServer") {
  server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start() {
  server_.start();
}

void EchoServer::onConnection(const starry::TcpConnectionPtr& conn) {
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const starry::TcpConnectionPtr& conn,
                           starry::Buffer* buf,
                           starry::Timestamp time) {
  std::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << std::format("{0:%F %T}", time);
  conn->send(msg);
}
