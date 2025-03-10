#include <chrono>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "echo.pb.h"
#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"
#include "rpc_server.h"

class EchoServiceImpl : public echo::EchoService {
 public:
  void Echo(const echo::EchoRequestPtr& request,
            const echo::EchoResponse* responsePrototype,
            const starry::RpcDoneCallback& done) override {
    echo::EchoResponse* response = responsePrototype->New();

    // 设置响应内容
    response->set_message(request->message());
    response->set_timestamp(getCurrentTimestamp());

    LOG_INFO << "Received Echo request: " << request->message();

    // 完成RPC调用
    done(response);
  }

  void EchoWithPrefix(const echo::EchoRequestPtr& request,
                      const echo::EchoResponse* responsePrototype,
                      const starry::RpcDoneCallback& done) override {
    // 创建响应对象
    echo::EchoResponse response;

    // 设置带前缀的响应内容
    response.set_message("PREFIX: " + request->message());
    response.set_timestamp(getCurrentTimestamp());

    LOG_INFO << "Received EchoWithPrefix request: " << request->message();

    // 完成RPC调用
    done(&response);
  }

 private:
  // 获取当前时间戳格式化为字符串
  std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    return std::format("{:%F %T}", now);
  }
};

int main(int argc, char* argv[]) {
  starry::Logger::setLogLevel(starry::LogLevel::INFO);
  LOG_INFO << "Starting Echo RPC Server...";

  // 设置RPC服务器地址和端口
  starry::EventLoop loop;
  starry::InetAddress listenAddr(8000);  // 监听端口8000

  // 创建RPC服务器
  starry::RpcServer server(&loop, listenAddr);

  // 设置线程数
  server.setThreadNum(4);

  // 创建并注册Echo服务实现
  EchoServiceImpl echoService;
  server.registerService(&echoService);

  // 启动服务器
  server.start();
  LOG_INFO << "Echo RPC Server started on port 8000";

  // 运行事件循环
  loop.loop();

  return 0;
}
