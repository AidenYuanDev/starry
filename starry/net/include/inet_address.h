#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <cmath>
#include <cstdint>
#include <string>
#include "sockets_ops.h"

namespace starry {

class InetAddress {
 public:
  // 监听本地端口
  explicit InetAddress(uint16_t port = 0,
                       bool loopbackOnly = false,
                       bool ipv6 = false);
  // 监听 IP::port
  InetAddress(std::string ip, uint16_t port, bool ipv6 = false);
  // ipv4 用 addr 直接初始化
  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}
  // ipv6 用 addr6 直接初始化
  explicit InetAddress(const struct sockaddr_in6& addr6) : addr6_(addr6) {}

  sa_family_t family() const { return addr_.sin_family; }  // 是 ipv4 还是 ipv6
  std::string toIp() const;                                // 返回 IP
  std::string toIpPort() const;                            // 返回 IP::port
  uint16_t port() const;                                   // 返回 port

  // ipv6 的设置和访问 addr6
  const struct sockaddr* getSockAddr() const {
    return sockets::sockaddr_cast(&addr6_);
  }
  void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

  // ipv4 的设置和访问 addr
  uint32_t ipv4NetEndian() const;
  uint16_t portNetEndian() const { return addr_.sin_port; }

  // 域名解析
  static bool resolve(std::string hostname, InetAddress* result);

  // 设置网卡端口
  void setScopeId(uint32_t scope_id);

 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace starry
