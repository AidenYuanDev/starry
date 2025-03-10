#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"
#include "tcp_client.h"
#include "rpc_channel.h"
#include "echo.pb.h"

// 回调处理封装
void onEchoResponse(const std::shared_ptr<echo::EchoResponse>& response) {
    LOG_INFO << "Received response: " << response->message()
             << " (timestamp: " << response->timestamp() << ")";
}

int main(int argc, char* argv[]) {
    starry::Logger::setLogLevel(starry::LogLevel::INFO);
    LOG_INFO << "Starting Echo RPC Client...";

    // 解析命令行参数
    std::string serverIp = "127.0.0.1";
    int serverPort = 8000;
    
    if (argc > 1) {
        serverIp = argv[1];
    }
    if (argc > 2) {
        serverPort = std::stoi(argv[2]);
    }

    // 设置RPC客户端
    starry::EventLoop loop;
    starry::InetAddress serverAddr(serverIp, serverPort);
    
    // 创建TCP客户端
    starry::TcpClient client(&loop, serverAddr, "EchoClient");
    
    // 创建RPC通道
    starry::RpcChannelPtr channel(new starry::RpcChannel);
    
    // 设置连接回调
    client.setConnectionCallback([&channel](const starry::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO << "Connected to server";
            channel->setConnection(conn);
        } else {
            LOG_INFO << "Disconnected from server";
            channel->setConnection(starry::TcpConnectionPtr());
        }
    });
    
    // 设置消息回调
    client.setMessageCallback(
        std::bind(&starry::RpcChannel::onMessage, channel.get(),
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
    // 连接服务器
    client.connect();
    
    // 测试循环
    std::thread testThread([&] {
        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 创建Echo服务存根
        echo::EchoService::Stub stub(channel.get());
        
        for (int i = 0; i < 5; ++i) {
            // 创建请求
            echo::EchoRequest request;
            request.set_message("Hello, RPC World! #" + std::to_string(i));
            
            LOG_INFO << "Sending Echo request: " << request.message();
            
            // 发送普通Echo请求
            stub.Echo(request, onEchoResponse);
            
            // 发送带前缀Echo请求
            stub.EchoWithPrefix(request, onEchoResponse);
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // 等待最后的响应
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 退出事件循环
        loop.quit();
    });
    
    // 运行事件循环
    loop.loop();
    
    // 等待测试线程完成
    testThread.join();
    
    LOG_INFO << "Echo RPC Client completed";
    
    return 0;
}
