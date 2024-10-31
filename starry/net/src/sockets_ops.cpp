#include <asm-generic/socket.h>
#include <endian.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "logging.h"
#include "sockets_ops.h"

using namespace starry;

// 地址强制转换
const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr) {
  return reinterpret_cast<const struct sockaddr*>(addr);
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr) {
  return reinterpret_cast<struct sockaddr*>(addr);
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr) {
  return reinterpret_cast<const struct sockaddr*>(addr);
}

const struct sockaddr_in* sockets::sockaddr_in_cast(
    const struct sockaddr* addr) {
  return reinterpret_cast<const struct sockaddr_in*>(addr);
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(
    const struct sockaddr* addr) {
  return reinterpret_cast<const struct sockaddr_in6*>(addr);
}

// 创建一个非阻塞的 socket | SOCK_NONBLOCK (非阻塞) | SOCK_CLOEXEC (执行子进程不持有 socket,防止子进程替换父 socket)
int sockets::createNonBlockingOrDie(sa_family_t famile) {
  int sockfd = ::socket(famile, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (sockfd < 0) {
    LOG_FATAL << "socket::createNonBlockingOrDie";
  }
  return sockfd;
}

// 绑定
void sockets::bindOrDie(int sockfd, const struct sockaddr* addr) {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (ret < 0) {
    LOG_FATAL << "sockets::bindOrDie";
  }
}

// 监听
void sockets::listenOrDir(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG_FATAL << "sockets::listenOrDir";
  }
}

// 接收连接
int sockets::accept(int sockfd, struct sockaddr_in6* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int saveErrno = errno;
    LOG_FATAL << "sockets::accept";
    switch (saveErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        errno = saveErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        LOG_FATAL << "unexpected error of ::accept " << saveErrno;
        break;
      default:
        LOG_FATAL << "unknow error of ::accept " << saveErrno;
        break;
    }
  }
  return connfd;
}

// 发起连接
int sockets::connect(int sockfd, const struct sockaddr* addr) {
  return ::connect(sockfd, addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

// 收取消息
ssize_t sockets::read(int sockfd, void* buf, size_t count) {
  return ::read(sockfd, buf, count);
}

// 收取消息, 填数组中
ssize_t sockets::readv(int sockfd, const struct iovec* iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

// 发送消息
ssize_t sockets::write(int sockfd, const void* buf, size_t count) {
  return ::write(sockfd, buf, count);
}

// 关闭 socket 
void sockets::close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_FATAL << "sockets::close";
  }
}

// shutdown 关闭发送消息
void sockets::shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOG_FATAL << "sockets::shutdown";
  }
}

// 转换成 IP:port
void sockets::toIpPort(char* buf, size_t size, const struct sockaddr* addr) {
  if (addr->sa_family == AF_INET6) {
    buf[0] = '[';
    toIp(buf + 1, size - 1, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
    uint16_t port = htobe16(addr6->sin6_port);
    assert(size > end);
    snprintf(buf + end, size - end, "]:%u", port);
  } else {
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = htobe16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
  }
}

// 转换成 IP
void sockets::toIp(char* buf, size_t size, const struct sockaddr* addr) {
  if (addr->sa_family == AF_INET6) {
    assert(size >= INET6_ADDRSTRLEN);
    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  } else {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  }
}

// ipv4 把字符格式地址转换成 IP:port ，存放到 addr
void sockets::fromIpPort(const char* ip,
                         uint16_t port,
                         struct sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = htobe16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG_FATAL << "sockets::fromIpPort";
  }
}

// ipv6 把字符格式地址转换成 IP:port ，存放到 addr
void sockets::fromIpPort(const char* ip,
                         uint16_t port,
                         struct sockaddr_in6* addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port = htobe16(port);
  if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
    LOG_FATAL << "sockets::fromIpPort";
  }
}

// 获取 socket 发生错误的原因
int sockets::getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

// 获取当前 sockaddr_in6
struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
  struct sockaddr_in6 localaddr;
  memset(&localaddr, 0, sizeof(localaddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    LOG_FATAL << "sockets::getLocalAddr";
  }
  return localaddr;
}

// 获取对方 sockaddr_in6
struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
  struct sockaddr_in6 peeraddr;
  memset(&peeraddr, 0, sizeof(peeraddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
    LOG_FATAL << "sockets::getPeerAddr";
  }
  return peeraddr;
}

// 是否是本地连接
bool sockets::isSelfConnect(int sockfd) {
  struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
  if (localaddr.sin6_family == AF_INET) {
    const struct sockaddr_in* laddr4 =
        reinterpret_cast<struct sockaddr_in*>(&localaddr);
    const struct sockaddr_in* raddr4 =
        reinterpret_cast<struct sockaddr_in*>(&peeraddr);
    return laddr4->sin_port == raddr4->sin_port &&
           laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
  } else if (localaddr.sin6_family == AF_INET6) {
    return localaddr.sin6_port == peeraddr.sin6_port &&
           memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr,
                  sizeof(localaddr.sin6_addr)) == 0;
  } else {
    return false;
  }
}
