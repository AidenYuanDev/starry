#include <iostream>
#include <string>
#include "echo_service.EchoService.rpc.h"  // 注意这是生成的头文件
#include "echo_service.pb.h"
#include "eventloop.h"
#include "rpc_server.h"

using namespace starry;
using namespace starry::examples;

// Echo 服务实现
class EchoServiceImpl : public EchoService {
 public:
  void Echo(::google::protobuf::RpcController* controller,
            const EchoRequest* request,
            EchoResponse* response,
            ::google::protobuf::Closure* done) override {
    std::cout << "Received request: " << request->message() << std::endl;

    // 简单地将请求消息回显到响应中
    response->set_message("Echo: " + request->message());

    // 完成调用
    done->Run();
  }
};

int main(int argc, char* argv[]) {
  // 默认端口
  uint16_t port = 8000;
  if (argc > 1) {
    port = static_cast<uint16_t>(std::stoi(argv[1]));
  }

  // 创建事件循环
  EventLoop loop;

  // 创建服务器地址
  InetAddress serverAddr(port);

  // 创建RPC服务器
  RpcServer server(&loop, serverAddr);

  // 创建并注册Echo服务
  EchoServiceImpl echoService;
  server.registerService(&echoService);

  std::cout << "Echo server starting on port " << port << "..." << std::endl;

  // 启动服务器
  server.start();

  // 运行事件循环
  loop.loop();

  return 0;
}
