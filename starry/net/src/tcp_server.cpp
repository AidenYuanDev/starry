#include <cassert>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <string>
#include "acceptor.h"
#include "callbacks.h"
#include "eventloop.h"
#include "eventloop_threadpool.h"
#include "inet_address.h"
#include "logging.h"
#include "sockets_ops.h"
#include "tcp_connection.h"
#include "tcp_server.h"

using namespace starry;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAdde,
                     const std::string& nameArg,
                     const Option option)
    : loop_(loop),
      ipPort_(listenAdde.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAdde, option == Option::kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      nextConnId_(1) {}

TcpServer::~TcpServer() {
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto& item : connections_) {
    TcpConnectionPtr conn(item.second);
    item.second.reset();
    conn->getLoop()->runInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
  }
}

// 设置线程数
void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

// 开启监听
void TcpServer::start() {
  if (!started_) {
    started_ = 1;
    threadPool_->start(threadInitCallback_);
    assert(!acceptor_->listening());
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

// 初始化新连接
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_++);
  std::string connName = name_ + buf;

  LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
           << connName << "] from " << peerAddr.toIpPort();

  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn(
      new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  connections_[buf] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

// 绑定 关闭回调函数
void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

// 直接调用 erase 删除conn 
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
