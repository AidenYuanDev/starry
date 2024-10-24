#pragma once

#include <arpa/inet.h>
#include <bits/types/struct_iovec.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstddef>
#include <cstdint>

namespace starry::sockets {

// socket 基本操作
int createNonBlockingOrDie(sa_family_t famile);
int connect(int sockfd, const struct sockaddr* addr);
void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDir(int sockfd);
int accept(int sockfd, struct sockaddr_in6* addr);
ssize_t read(int sockfd, void* buf, size_t count);
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t write(int sockfd, const void* buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toIpPort(char* buf, size_t size, const struct sockaddr* addr);  // 转换成 IP::port
void toIp(char* buf, size_t size, const struct sockaddr* addr);      // 转换成IP

// 从字符串初始化 addr
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

int getSocketError(int sockfd);  // 获取 socket 的错误原因

// 地址转换
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

struct sockaddr_in6 getLocalAddr(int sockfd);  // 获取当前地址
struct sockaddr_in6 getPeerAddr(int sockfd);   // 获取对方地址
bool isSelfConnect(int sockfd);                // 判断连接的是不是本地

}  // namespace starry::sockets
