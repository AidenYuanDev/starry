#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include "buffer.h"
#include "callbacks.h"
#include "eventloop.h"
#include "inet_address.h"
#include "socket.h"

struct tcp_info;

namespace starry {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop,
                const std::string& nameArg,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();
  // 获取 当前 loop
  EventLoop* getLoop() const { return loop_; }
  // 获取绑定的本地地址
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() const { return localAddr_; }
  // 获取绑定的对方地址
  const InetAddress& peerAddress() const { return peerAddr_; }
  // 是否已经建立连接
  bool connected() const { return state_ == StateE::kConnected; }
  // 是否已经关闭连接
  bool disconnected() const { return state_ == StateE::kDisconnected; }

  bool getTcpInfo(struct tcp_info*) const;  // 获取 tcp 的信息
  std::string getTcpInfoString() const;  // 获取 tcp 的信息以 string 的形式返回

  void send(const void* message, int len);     // 发送消息
  void send(const std::string_view& message);  // 发送消息
  void send(Buffer* message);                  // 发送消息
  void shutdown();                             // 半连接：只读不写
  void forceClose();                           // 强制关闭
  void forceCloseWithDelay(double seconds);    // 延时关闭
  void setTcpNoDelay(bool on);                 // 是否开启 nagle 算法
  void startRead();                            // 开启读
  void stopRead();                             // 停止读
  bool isReading() const { return reading_; }  // 是否正在读

  // 设置连接回调
  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }
  // 设置消息回调
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  // 设置写回调
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }
  // 设置高水位标记回调
  void setHigWaterMarkCallback(const HighWaterMarkCallback& cb,
                               size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  Buffer* inputBuffer() { return &inputBuffer_; }    // 读缓冲区
  Buffer* outputBuffer() { return &outputBuffer_; }  // 写缓冲区

  void setCloseCallback(const CloseCallback& cb) {
    closeCallback_ = cb;
  }  // 设置关闭回调

  void connectEstablished();  // 开启连接
  void connectDestroyed();    // 关闭连接

 private:
  enum class StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void handleRead(Timestamp receiveTime);  // 处理读
  void handleWrite();                      // 处理写
  void handleClose();                      // 处理关闭
  void handleError();                      // 处理错误

  void sendInLoop(const std::string_view& message);  // 向 loop 发送 message
  void sendInLoop(const void* message, size_t len);  // 像 loop 发送 message
  void shutdownInLoop();                             // 半连接：只读不写
  void forceCloseInLoop();                           // 强制关闭 loop
  void setState(StateE s) { state_ = s; }            // 设置状态标志
  const char* stateToString() const;                 // 打印现在的状态
  void startReadInLoop();                            // 开启读
  void stopReadInLoop();                             // 停止读

  EventLoop* loop_;                              // 所持有的 loop
  const std::string name_;                       // loop name
  std::atomic<StateE> state_;                    // 现在的状态
  bool reading_;                                 // 是否在读
  std::unique_ptr<Socket> socket_;               // 操作 socket fd
  std::unique_ptr<Channel> channel_;             // fd 对应的channel
  const InetAddress localAddr_;                  // 本地地址
  const InetAddress peerAddr_;                   // 对方地址
  ConnectionCallback connectionCallback_;        // 连接回调
  MessageCallback messageCallback_;              // 消息回调
  WriteCompleteCallback writeCompleteCallback_;  // 写回调
  HighWaterMarkCallback highWaterMarkCallback_;  // 高水位标记回调
  CloseCallback closeCallback_;                  // 关闭回调
  size_t highWaterMark_;                         // 高水位线
  Buffer inputBuffer_;                           // 读缓冲区
  Buffer outputBuffer_;                          // 写缓冲区
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

}  // namespace starry
