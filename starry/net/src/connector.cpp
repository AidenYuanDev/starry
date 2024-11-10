#include "channel.h"
#include "connector.h"
#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"
#include "sockets_ops.h"

#include <algorithm>
#include <errno.h>
#include <cassert>
#include <functional>

using namespace starry;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(States::kDisconnected),
      retryDelayMs_(kInitRetryDelayMs) {
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
  LOG_DEBUG << "dtor[" << this << "]";
  assert(!channel_);
}

// 把开启循环回调函数 startInLoop 绑定到 loop 中
void Connector::start() {
  connect_ = true;
  loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

// 开启连接
void Connector::startInLoop() {
  loop_->assertInLoopThread();
  assert(state_ == States::kDisconnected);
  if (connect_) {
    connect();
  } else {
    LOG_DEBUG << "do not connect";
  }
}

// 绑定停止回调
void Connector::stop() {
  connect_ = false;
  loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

// 设置连接停止状态， 并从 channel 中移除 fd
void Connector::stopInLoop() {
  loop_->assertInLoopThread();
  if (state_ == States::KConnecting) {
    setState(States::KConnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}

// 建立连接
void Connector::connect() {
  int sockfd = sockets::createNonBlockingOrDie(serverAddr_.family());
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_FATAL << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      break;

    default:
      LOG_FATAL << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

// 设置状态， 延迟， 开启连接
void Connector::restart() {
  loop_->assertInLoopThread();
  setState(States::kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

// 设置写回调，异常回调，开启 channel 的写
void Connector::connecting(int sockfd) {
  setState(States::KConnecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
  channel_->setErrorCallback(std::bind(&Connector::handleError, this));
  channel_->enableWriting();
}

// channel 的日常关闭流程，绑定 resetChannel
int Connector::removeAndResetChannel() {
  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
  return sockfd;
}

// 重新设置 channel
void Connector::resetChannel() {
  channel_.reset();
}

// 写回调 -- 处理写可能发生的异常状态
void Connector::handleWrite() {
  LOG_TRACE << "Connector::handleWrite " << stateToString(state_);

  if (state_ == States::KConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err) {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err << " "
               << strerror_tl(err);
      retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    } else {
      setState(States::KConnected);
      if (connect_) {
        newConnectionCallback_(sockfd);
      } else {
        sockets::close(sockfd);
      }
    }
  } else {
    assert(state_ == States::kDisconnected);
  }
}

// 异常回调 -- 处理异常时，不在 KConnecting 状态的情况
void Connector::handleError() {
  LOG_ERROR << "Connector::handleError state=" << stateToString(state_);
  if (state_ == States::KConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
  }
}

// 等定时器实现
void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(States::kDisconnected);
  if (connect_) {
    LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
             << " in " << retryDelayMs_ << " milliseconds. ";
    loop_->runAfter(retryDelayMs_ / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  } else {
    LOG_DEBUG << "do not connect";
  }
}

std::string Connector::stateToString(States state) {
  switch (state) {
    case States::kDisconnected:
      return "kDisconnected";
    case States::KConnecting:
      return "KConnecting";
    case States::KConnected:
      return "KConnected";
    default:
      return "invalid state";
  }
}
