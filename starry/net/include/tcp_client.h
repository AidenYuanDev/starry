#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include "callbacks.h"
#include "eventloop.h"
#include "inet_address.h"
#include "tcp_connection.h"

namespace starry {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient {
 public:
  TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const std::string& nameArg);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }

  const std::string& name() const { return name_; }

  void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }

  void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }

  void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
 
 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_;
  const std::string name_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;
  bool connect_;
  int nextConnId_;
  mutable std::mutex mutex_;
  TcpConnectionPtr connection_;
};

}  // namespace starry
