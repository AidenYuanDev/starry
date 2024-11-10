#include "channel.h"
#include "eventloop.h"
#include "logging.h"

#include <cassert>
#include <chrono>
#include <string>
#include <strings.h>
#include <sstream>
#include <sys/epoll.h>
#include <csignal>
#include <poll.h>

using namespace starry;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false) {}

Channel::~Channel() {
  assert(!eventHandling_);
  assert(!addedToLoop_);
  if (loop_->isInLoopThread()){
    assert(!loop_->hasChannel(this));
  }
}

void Channel::tie(const std::shared_ptr<void>& obj){
  tie_ = obj;
  tied_=true;
}

void Channel::update() {
  addedToLoop_ = true;
  loop_->updateChannel(this);
}

void  Channel::remove() {
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
  std::shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
  eventHandling_ = true;
  LOG_TRACE << reventsToString();
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    if (closeCallback_) closeCallback_();
  }

  if (revents_ & EPOLLERR) {
    if (errorCallback_) errorCallback_();
  }

  if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (readCallback_) readCallback_(receiveTime);
  }

  if (revents_ & EPOLLOUT) {
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

std::string Channel::reventsToString() const {
  return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const {
  return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & EPOLLIN)
    oss << "IN ";
  if (ev & EPOLLPRI)
    oss << "PRI ";
  if (ev & EPOLLOUT) 
    oss << "OUT ";
  if (ev & EPOLLHUP)
    oss << "HUP ";
  if (ev & EPOLLERR)
    oss << "ERR ";
  
  return oss.str();
}
