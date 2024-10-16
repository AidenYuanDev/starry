#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>

namespace starry {

class EventLoop;

class Channel {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback =
      std::function<void(std::chrono::steady_clock::time_point)>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  // 设置回调
  void handleEvent(std::chrono::steady_clock::time_point receiveTime); // 定时回调
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); } // 读回调 
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); } // 写回调
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); } // 关闭回调
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); } // 错误回调
  
  // 保证TcpConnection在handleEvent函数中不会被销毁
  void tie(const std::shared_ptr<void>&);

  // 由上层EventLoop调用,查看和控制Channel的成员变量
  int fd() const { return fd_; } 
  int events() const {return events_; }
  void setRevent(int revents) { revents_ = revents; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }
  EventLoop* ownerLoop() { return loop_;}
  void remove();
  void enableReading() { events_ |= kReadEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const {return events_ & kReadEvent; }

  // 同层epool调用
  int index() { return index_; }
  void setIndex(int idx) { index_ = idx; }

  // 方便调试
  std::string reventsToString() const;
  std::string eventsToString() const;

 private:
  static std::string eventsToString(int fd, int ev);

  void update();
  void handleEventWithGuard(std::chrono::steady_clock::time_point receiveTime);

  // 文件描述符事件
  static const int kNoneEvent;   // 无事件
  static const int kReadEvent;   // 读事件
  static const int kWriteEvent;  // 写事件

  EventLoop* loop_;  // 该Channel符所属的 loop
  const int fd_;     // 当前Channel的文件描述符
  int events_;       // 监测的事件
  int revents_;      // epoll触发的事件
  int index_;        // 代表Channel状态，epoll设定了kNew,kAdded,kDeleted

  std::weak_ptr<void> tie_;  // 使用shared_ptr保证TcpConnection的生命周期
  bool tied_;                // 是否成功绑定TcpConnection
  bool eventHandling_;       // 是否正在处理文件描述符
  bool addedToLoop_;         // 是否加入到loop
  ReadEventCallback readCallback_;  // 读回调
  EventCallback writeCallback_;     // 写回调
  EventCallback closeCallback_;     // 关闭回调
  EventCallback errorCallback_;     // 错误回调
};

}  // namespace starry
