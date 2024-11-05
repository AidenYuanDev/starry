#include "callbacks.h"
#include "eventloop.h"
#include "eventloop_threadpool.h"
#include "inet_address.h"
#include "tcp_connection.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace starry {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  enum class Option {  // 选项的状态
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option option = Option::kNoReusePort);
  ~TcpServer();

  const std::string& ipPort() const { return ipPort_; }
  EventLoop* getLoop() const { return loop_; }
  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb) {
    threadInitCallback_ = cb;
  }
  std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

  void start();

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }

 private:
  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

  void newConnection(int sockfd, const InetAddress& peerAddr);
  void removeConnection(const TcpConnectionPtr& conn);
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  EventLoop* loop_;                                  // acceptor 的 loop
  const std::string ipPort_;                         // ip:port
  const std::string name_;                           // loop name
  std::unique_ptr<Acceptor> acceptor_;               // accept
  std::shared_ptr<EventLoopThreadPool> threadPool_;  // 线程池
  ConnectionCallback connectionCallback_;            // 连接回调函数
  MessageCallback messageCallback_;                  // 消息回调函数
  WriteCompleteCallback writeCompleteCallback_;      // 写回调函数
  ThreadInitCallback threadInitCallback_;            // 线程初始化函数
  std::atomic<int32_t> started_;                     // 是否开启
  int nextConnId_;                                   // 下一个 EventLoop
  ConnectionMap connections_;                        // 连接 map
};

}  // namespace starry
