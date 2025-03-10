#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include "buffer.h"
#include "callbacks.h"
#include "channel.h"
#include "eventloop.h"
#include "inet_address.h"
#include "logging.h"
#include "socket.h"
#include "sockets_ops.h"
#include "tcp_connection.h"

using namespace starry;

// 默认连接回调
void starry::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
}
// 默认写回调
void starry::defaultMessageCallback(const TcpConnectionPtr&,
                                    Buffer* buffer,
                                    Timestamp) {
  buffer->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(nameArg),
      state_(StateE::kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64 * 1024 * 1024) {
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
            << " fd=" << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd() << " state=" << stateToString();
  assert(state_ == TcpConnection::StateE::kDisconnected);
}

// 获取 tcp 信息
bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const {
  return socket_->getTcpInfo(tcpi);
}

// 获取 tcp 信息，以 string 形式返回
std::string TcpConnection::getTcpInfoString() const {
  char buf[1024];
  buf[0] = '\0';
  socket_->getTcpInfoString(buf, sizeof(buf));
  return buf;
}

// 发送 data
void TcpConnection::send(const void* data, int len) {
  send(std::string_view(static_cast<const char*>(data), len));
}

// 把 TcpConnection::sendInLoop 作为函数指针绑定到 runInLoop 中执行
void TcpConnection::send(const std::string_view& message) {
  if (state_ == StateE::kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      void (TcpConnection::*fp)(const std::string_view& message) =
          &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, this, message));
    }
  }
}

// 把 TcpConnection::sendInLoop 作为函数指针绑定到 runInLoop 中执行
void TcpConnection::send(Buffer* buf) {
  if (state_ == StateE::kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    } else {
      void (TcpConnection::*fp)(const std::string_view& message) =
          &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
    }
  }
}

// 调用重载版本，向 channel 中写入消息
void TcpConnection::sendInLoop(const std::string_view& message) {
  sendInLoop(message.data(), message.size());
}

// 向 channel 中写入消息
void TcpConnection::sendInLoop(const void* data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;  // 剩余未发送的字节数
  bool faultError = false;
  if (state_ == StateE::kDisconnected) {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = sockets::write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_) {
        loop_->queueInLoop(
            std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_FATAL << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) {
          faultError = true;
        }
      }
    }
  }

  // 如果一个发不完，就放到 outputBuffer_ 中，让 handleWrite
  // 自动触发完成发送工作
  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ &&
        highWaterMarkCallback_) {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(),
                                   oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

// 半连接，状态切换成 kDisconnecting, 并把 shutdownInLoop 绑定到 runInLoop
void TcpConnection::shutdown() {
  if (state_ == StateE::kConnected) {
    setState(StateE::kDisconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,
                               this));  // 保证关闭操作的顺序性
  }
}

// 半连接，回调函数
void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting()) {
    socket_->shutdownWrite();
  }
}

// 状态切换成 kDisconnecting, 并把 forceCloseInLoop 绑定到 queueInLoop
void TcpConnection::forceClose() {
  if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
    setState(StateE::kDisconnecting);
    // 保证关闭操作的顺序性
    loop_->queueInLoop(
        std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

// 强制关闭回调
void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
    handleClose();
  }
}

// 返回现在的状态
const char* TcpConnection::stateToString() const {
  switch (state_) {
    case StateE::kDisconnected:
      return "kDisconnected";
    case StateE::kConnecting:
      return "kConnecting";
    case StateE::kConnected:
      return "kConnected";
    case StateE::kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

// 设置 naggle 算法
void TcpConnection::setTcpNoDelay(bool on) {
  socket_->setTcpNoDelay(on);
}

// 开启读
void TcpConnection::startRead() {
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

// 读回调
void TcpConnection::startReadInLoop() {
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading()) {
    channel_->enableReading();
    reading_ = true;
  }
}

// 停止读
void TcpConnection::stopRead() {
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

// 停止读回调
void TcpConnection::stopReadInLoop() {
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading()) {
    channel_->disableReading();
    reading_ = false;
  }
}

// 连接建立
void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == StateE::kConnecting);
  setState(StateE::kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

// 连接断开
void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  if (state_ == StateE::kConnected) {
    setState(StateE::kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

// 处理读
void TcpConnection::handleRead(Timestamp receiveTime) {
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

// 处理写
void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = sockets::write(channel_->fd(), outputBuffer_.peek(),
                               outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          loop_->queueInLoop(
              std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == StateE::kDisconnecting) {
          shutdownInLoop();
        }
      }
    } else {
      LOG_FATAL << "TcpConnection::handleWrite";
    }
  } else {
    LOG_TRACE << "Connection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}

// 处理关闭
void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
  setState(StateE::kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  closeCallback_(guardThis);
}

// 处理错误
void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
