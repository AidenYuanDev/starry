#pragma once

#include <map>
#include <vector>
#include "channel.h"
#include "eventloop.h"

class Channel;

namespace starry {

class EpollPoller {
 public:
  using ChannelList = std::vector<Channel*>;
  EpollPoller(EventLoop* loop);
  ~EpollPoller();

  Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel) const;
  void assertInLoopThread() const {  // 断言是否在同一个线程
    ownerLoop_->assertInLoopThread();
  }

 private:
  using ChannelMap = std::map<int, Channel*>;
  using EventList = std::vector<struct epoll_event>;

  // 调试
  static const std::string operationToString(int op);

  // 操作 epoll
  void update(int operation, Channel* channel);
  // 把活跃的事件放到 activeChannels
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  static const int kInitEventListSize = 16;  // 初始 epoll 监听的文件数

  EventLoop* ownerLoop_;  // 所属的 EventLoop
  int epollfd_;           // 创建的epollfd
  EventList events_;      // 监听的fd
  ChannelMap channels_;   // fd 对应 C
};

}  // namespace starry
