#include <gtest/gtest.h>
#include "inet_address.h"
#include "socket.h"

using namespace starry;

class SocketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // 可以在这里进行一些初始化
  }

  void TearDown() override {
    // 清理资源
  }
};

// 1. 基本功能测试
TEST_F(SocketTest, Creation) {
  // 创建非阻塞套接字
  Socket socket(
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
  ASSERT_TRUE(socket.fd() > 0);
}

// 2. 设置选项测试
TEST_F(SocketTest, SetOptions) {
  Socket socket(
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));

  // 测试设置/获取各种socket选项
  socket.setReuseAddr(true);
  socket.setReusePort(true);
  socket.setKeepAlive(true);

  // 可以通过getsockopt验证设置是否成功
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);
  ::getsockopt(socket.fd(), SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
  EXPECT_EQ(optval, 1);
}

// 3. 绑定和监听测试
TEST_F(SocketTest, BindAndListen) {
  Socket socket(
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
  InetAddress addr(0);  // 使用随机端口

  socket.bindAddress(addr);
  socket.listen();

  // 获取实际绑定的地址
  struct sockaddr_in local;
  socklen_t addrlen = static_cast<socklen_t>(sizeof local);
  ASSERT_EQ(::getsockname(socket.fd(),
                          reinterpret_cast<struct sockaddr*>(&local), &addrlen),
            0);
  EXPECT_NE(ntohs(local.sin_port), 0);  // 确保分配了端口
}

// 4. 关闭行为测试
// TEST_F(SocketTest, ShutdownWrite) {
//     Socket socket(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK |
//     SOCK_CLOEXEC, 0)); socket.shutdownWrite();
//
//     // 验证写入端已关闭
//     char buffer[1];
//     EXPECT_EQ(::write(socket.fd(), buffer, 1), -1);
//     EXPECT_EQ(errno, EPIPE);
// }

// 5. 错误处理测试
TEST_F(SocketTest, ErrorHandling) {
  Socket socket(
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
  InetAddress addr("127.0.0.1", 1);  // 使用受限端口

  // 期望绑定失败
  EXPECT_DEATH(socket.bindAddress(addr), "");
}

// 6. 连接测试（需要配对的服务器和客户端）
TEST_F(SocketTest, Connect) {
  // 创建服务器socket
  Socket server(
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
  InetAddress serverAddr(0);  // 随机端口
  server.bindAddress(serverAddr);
  server.listen();

  // 获取实际的服务器地址
  struct sockaddr_in local;
  socklen_t addrlen = static_cast<socklen_t>(sizeof local);
  ASSERT_EQ(::getsockname(server.fd(),
                          reinterpret_cast<struct sockaddr*>(&local), &addrlen),
            0);
  uint16_t port = ntohs(local.sin_port);

  // 创建客户端socket
  Socket client(
      ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
  InetAddress clientAddr("127.0.0.1", port);

  // 尝试连接
  int ret = ::connect(client.fd(), clientAddr.getSockAddr(),
                      sizeof(struct sockaddr_in));
  EXPECT_TRUE(ret == 0 || errno == EINPROGRESS);
}
