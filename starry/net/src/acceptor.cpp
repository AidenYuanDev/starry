#include "acceptor.h"
#include "logging.h"
#include "eventloop.h"
#include "inet_address.h"
#include "sockets_ops.h"
#include <cassert>
#include <cerrno>
#include <functional>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace starry;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport) :
  loop_(loop),
  acceptSocket_(sockets::createNonBlockingOrDie(listenAddr.family())),
  acceptChannel_(loop, acceptSocket_.fd()),
  listening_(false),
  idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);
}

// fd 启动监听，并允许读
void Acceptor::listen() {
  loop_->assertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

// 处理连接
void Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    } else {
      sockets::close(connfd);
    }
  } else {
    LOG_ERROR << "in Acceptor::handleRead";
    if (errno == EMFILE) {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), nullptr, nullptr);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}
