#pragma once

#include "channel.h"
#include "eventloop.h"
#include "inet_address.h"

#include <functional>
#include <memory>

namespace starry {

class Channel;
class EventLoop;

class Connector : public std::enable_shared_from_this<Connector> {
 public:
  using NewConnectionCallback = std::function<void(int sockfd)>;

  Connector(EventLoop* loop, const InetAddress& serverAddr);
  ~Connector();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }

  void start();    // 开始
  void restart();  // 重新1开始
  void stop();     // 停止

  // 服务地址
  const InetAddress& serverAddress() const { return serverAddr_; }

 private:
  enum class States {
    kDisconnected,
    KConnecting,
    KConnected,
  };
  static const int kMaxRetryDelayMs = 30 * 1000;
  static const int kInitRetryDelayMs = 500;

  void setState(States s) { state_ = s; }  // 设置状态
  void startInLoop();                      // 开始回调
  void stopInLoop();                       // 停止回调
  void connect();                          // 建立连接
  void connecting(int sockfd);             // channel 连接
  void handleWrite();                      // 处理写
  void handleError();                      // 处理错误
  void retry(int sockfd);                  // 延时开始
  int removeAndResetChannel();  // 移除 channel + 重新设置 channel
  void resetChannel();          // 重新设置 channel

  EventLoop* loop_;                              // 所持有的 loop
  InetAddress serverAddr_;                       // 服务端地址
  bool connect_;                                 // 是否连接
  States state_;                                 // 连接状态
  std::unique_ptr<Channel> channel_;             // 所持有的 channel
  NewConnectionCallback newConnectionCallback_;  // 新连接回调
  int retryDelayMs_;                             // 延时
};

}  // namespace starry
