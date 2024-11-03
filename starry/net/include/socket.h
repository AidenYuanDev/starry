#pragma once

#include <cstddef>
#include "inet_address.h"
struct tcp_info;

namespace starry {

class InetAddress;

class Socket {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int fd() const { return sockfd_; }        // 获得 fd
  bool getTcpInfo(struct tcp_info*) const;  // 获得 tcp 信息
  bool getTcpInfoString(char* buf, size_t len) const;

  void bindAddresse(const InetAddress& localaddr);  // 绑定地址
  void listen();                                    // 监听连接
  int accpet(InetAddress* peeraddr);                // 接收连接

  void shutdownWrite();         // 半连接，只收取不发送
  void setTcpNoDelay(bool on);  //  是否开启 nagle 算法
  void setReuseAddr(bool on);   // 是否开启端口复用
  void setReusePort(bool on);   // 是否开启端口复用
  void setKeepAlive(bool on);   // 是否开\开启心跳检测

 private:
  const int sockfd_;
};

}  // namespace starry
