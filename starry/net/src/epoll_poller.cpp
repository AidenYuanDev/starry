#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstring>

#include "channel.h"
#include "epoll_poller.h"
#include "eventloop.h"
#include "logging.h"
#include "poller.h"

using namespace starry;

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_FATAL << "EPollPoller::EPollPoller";
  }
}

EpollPoller::~EpollPoller() {
  ::close(epollfd_);
}

// 把监听到的活跃文件描述符通过 fillActiveChannels 放到 activeChannels中
std::chrono::steady_clock::time_point EpollPoller::poll(
    int timeoutMs,
    ChannelList* activeChannels) {
  LOG_TRACE << "fd total count " << channels_.size();
  int numEvents = ::epoll_wait(epollfd_, events_.data(),
                               static_cast<int>(events_.size()), timeoutMs);

  int savedErron = errno;
  auto now = std::chrono::steady_clock::now();
  if (numEvents > 0) {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if (static_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOG_TRACE << "nothing happened";
  } else {
    if (savedErron != EINTR) {
      errno = savedErron;
      LOG_FATAL << "EPollPoller::poll()";
    }
  }
  return now;
}

void EpollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const {
  assert(static_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; i++) {
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->setRevent(events_[i].events);
    activeChannels->push_back(channel);
  }
}

// kNew 加入 events_, kDeleted 判断有没有删干净， kAdded
// 没有事件就删除fd，有事件就修改
void EpollPoller::updateChannel(Channel* channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd() << "events = " << channel->events()
            << " index = " << index;
  int fd = channel->fd();
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }
  } else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

// 把 kAdded 或 kDeleted 从events_中删除，并还原成kNew
void EpollPoller::removeChannel(Channel* channel) {
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

// 把 channel 转换成 event 用 epoll_ctl 执行对 fd 的operation
void EpollPoller::update(int operation, Channel* channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
            << " fd = " << fd
            << " event = { " << channel->eventsToString() << " }";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    LOG_FATAL << "epoll_ctl op =" << operationToString(operation) << " fd " << fd;
  }
}

// 把 operation 转成 string 
const std::string EpollPoller::operationToString(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}
