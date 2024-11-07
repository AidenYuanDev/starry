#pragma once

#include <vector>
#include "channel.h"
#include "eventloop.h"
#include "poller.h"

namespace starry {

class EpollPoller : public Poller {
 public:
  EpollPoller(EventLoop* loop);
  ~EpollPoller() override;

  Timestamp poll(
      int timeoutMs,
      ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
  using EventList = std::vector<struct epoll_event>;

  // 调试
  static const std::string operationToString(int op);

  // 操作 epoll
  void update(int operation, Channel* channel);
  // 把活跃的事件放到 activeChannels
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  static const int kInitEventListSize = 16;  // 初始 epoll 监听的文件数

  int epollfd_;       // 创建的epollfd
  EventList events_;  // 监听的fd
};

}  // namespace starry
