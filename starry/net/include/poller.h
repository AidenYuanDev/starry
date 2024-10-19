#pragma once

#include <chrono>
#include <map>
#include <vector>
#include "eventloop.h"

namespace starry {

class Channel;

class Poller {
 public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* loop);
  virtual ~Poller();

  // 操作 Channel 
  virtual std::chrono::steady_clock::time_point poll (int timeoutMs, ChannelList* activeChannells) = 0;
  virtual void updateChannel(Channel* channel) = 0;
  virtual void removeChannel(Channel* channel) = 0;

  // 调试
  virtual bool hasChannel( Channel* channel) const;  // 判断当前 Poller 是否有 Channel
  static Poller* newDefaultPoller(EventLoop* loop);  // 设置默认Poller = epoll
  void assertInLoopThread() const {                        // 断言是否在同一个线程
    ownerLoop_->assertInLoopThread();
  } 

 protected:
  using ChannelMap = std::map<int, Channel*>;
  ChannelMap channels_;  // fd 对应 Channel

 private:
  EventLoop* ownerLoop_;  // 所属的 EventLoop
};

}  // namespace starry
