#include <iostream>
#include <string>
#include "echo_service.EchoService.rpc.h"  // 注意这是生成的头文件
#include "echo_service.pb.h"
#include "eventloop.h"
#include "rpc_channel.h"
#include "rpc_closure.h"
#include "rpc_controller.h"
#include "tcp_client.h"

using namespace starry;
using namespace starry::examples;

int main(int argc, char* argv[]) {
  // 默认参数
  std::string serverIp = "127.0.0.1";
  uint16_t serverPort = 8000;
  std::string message = "Hello, RPC!";

  // 解析命令行参数
  if (argc > 1)
    serverIp = argv[1];
  if (argc > 2)
    serverPort = static_cast<uint16_t>(std::stoi(argv[2]));
  if (argc > 3)
    message = argv[3];

  // 创建事件循环
  EventLoop loop;

  // 创建服务器地址
  InetAddress serverAddr(serverIp, serverPort);

  // 创建TCP客户端
  TcpClient client(&loop, serverAddr, "EchoClient");

  // 连接标志和连接对象
  bool connected = false;
  TcpConnectionPtr conn;

  // 设置连接回调
  client.setConnectionCallback([&](const TcpConnectionPtr& connection) {
    if (connection->connected()) {
      std::cout << "Connected to server" << std::endl;
      connected = true;
      conn = connection;
    } else {
      std::cout << "Disconnected from server" << std::endl;
      connected = false;
      conn.reset();
      loop.quit();
    }
  });

  // 连接服务器
  client.connect();

  // 等待连接建立
  while (!connected) {
    loop.runAfter(0.01, [&loop]() { loop.quit(); });
    loop.loop();
  }

  if (!connected) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }

  // 创建RPC通道
  RpcChannel channel(conn);

  // 创建服务存根
  EchoService_Stub stub(&channel);

  // 创建请求
  EchoRequest request;
  request.set_message(message);

  // 创建响应对象
  EchoResponse response;

  // 创建控制器
  RpcController controller;

  // 调用完成标志
  bool done = false;

  std::cout << "Sending message: " << message << std::endl;

  // 发起RPC调用
  stub.Echo(&controller, &request, &response, NewCallback([&]() {
    std::cout << "Received response: " << response.message() << std::endl;
    done = true;
    conn->forceClose();  // 调用完成后关闭连接
  }));

  // 等待调用完成
  while (!done) {
    loop.loop();
  }

  return 0;
}
