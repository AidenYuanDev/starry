#include "echo.h"
#include "inet_address.h"
#include "logging.h"
#include "eventloop.h"
#include <unistd.h>

int main() {
  starry::Logger::setLogLevel(starry::LogLevel::FATAL);
  LOG_INFO << "pid = " << getpid();

  starry::EventLoop loop;
  starry::InetAddress listenAddr(2007);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
  return 0;
}
