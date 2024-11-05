#pragma once

#include <functional>
#include "channel.h"
#include "eventloop.h"
#include "inet_address.h"
#include "socket.h"

namespace starry {

class EventLoop;
class InetAddress;

class Acceptor {
 public:
  using NewConnectionCallback =
      std::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  // 设置连接回调
  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }
  void listen();                                 // 开启监听
  bool listening() const { return listening_; }  // 是否正在监听

 private:
  void handleRead();

  EventLoop* loop_;        // 绑定的 loop
  Socket acceptSocket_;    // socket 持有 fd
  Channel acceptChannel_;  // fd 持有的 channel
  NewConnectionCallback newConnectionCallback_;
  bool listening_;  // 是否正在监听
  int idleFd_;      // 预留一个文件描述符，防止文件描述符耗尽
};

}  // namespace starry
