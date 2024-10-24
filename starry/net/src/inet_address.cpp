#include "inet_address.h"
#include "logging.h"
#include "sockets_ops.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace starry;

// 给端口，监听所有地址
InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6) {
  if (ipv6) {
    memset(&addr6_, 0, sizeof( addr6_));
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = htobe16(portArg);
  } else {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = htobe32(ip);
    addr_.sin_port = htobe16(portArg);
  }
}

// 监听指定端口和 IP 地址
InetAddress::InetAddress(std::string ip, uint16_t portArg, bool ipv6) {
  if (ipv6 || ip.find(':') != std::string::npos) {
    memset(&addr6_, 0, sizeof(addr6_));
    sockets::fromIpPort(ip.c_str(), portArg, &addr6_);
  } else {
    memset(&addr_, 0, sizeof(addr_));
    sockets::fromIpPort(ip.c_str(), portArg, &addr_);
  }
}

// 返回 IP::port 的形式
std::string InetAddress::toIpPort() const {
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof(buf), getSockAddr());
  return buf;
}

// 返回 IP 的形式
std::string InetAddress::toIp() const {
  char buf[64] = "";
  sockets::toIp(buf, sizeof(buf), getSockAddr());
  return buf;
}


// ipv4 网络字节序
uint32_t InetAddress::ipv4NetEndian() const {
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

// 返回端口号
uint16_t InetAddress::port() const {
  return be16toh(portNetEndian());
}

// 返回的 IP
static thread_local char t_resolveBuffer[64 * 1024];
// 域名解析
bool InetAddress::resolve(std::string hostname, InetAddress *out) {
  assert(out != nullptr);
  struct hostent hent;
  struct hostent* he = nullptr;
  int herrno = 0;
  memset(&hent, 0, sizeof(hent));

  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof(t_resolveBuffer), &he, & herrno);
  if (ret == 0 && he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  } else {
    if (ret) {
      LOG_FATAL << "InetAddress::resolve";
    }
    return false;
  }
}

void InetAddress::setScopeId(uint32_t scope_id) {
  if (family() == AF_INET6) {
    addr6_.sin6_scope_id = scope_id;
  }
}
