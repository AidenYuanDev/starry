#include "callbacks.h"
#include "channel.h"
#include "resolver.pb.h"

#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"
#include "rpc_channel.h"
#include "tcp_client.h"
#include "tcp_connection.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>

using namespace starry;

class RpcClient {
 public:
  RpcClient(EventLoop* loop, const InetAddress& serverAddr)
      : loop_(loop),
        client_(loop, serverAddr, "RpcClient"),
        channel_(new RpcChannel),
        got_(0),
        total_(0),
        stub_(channel_.get()) {
    client_.setConnectionCallback(
        std::bind(&RpcClient::onConnection, this, std::placeholders::_1));
    client_.setMessageCallback(
        std::bind(&RpcChannel::onMessage, channel_.get(), std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
  }

  void connect() { client_.connect(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
      channel_->setConnection(conn);
      total_ = 4;
    }
  }

  void resolve(const std::string& host) {
    resolver::ResolveRequest request;
    request.set_address(host);

    stub_.Resolve(request, std::bind(&RpcClient::resolved, this,
                                     std::placeholders::_1, host));
  }

  void resolved(const resolver::ResolveRequestPtr& resp, std::string host) {

  }

  EventLoop* loop_;
  TcpClient client_;
  RpcChannelPtr channel_;
  int got_;
  int total_;
  resolver::ResolverService::Stub stub_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << " pid = " << getppid();
  if (argc > 1) {
    EventLoop loop;
    InetAddress serverAddr(argv[1], 2053);

    // RpcClient rpcClient(&loop, serverAddr);
  }
}
