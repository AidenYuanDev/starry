#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include "eventloop.h"

namespace starry {

class EventLoop;

class EventLoopThread {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();

  EventLoop* startLoop();

 private:
  void threadFunc();

  std::mutex mutex_;              // 线程锁
  EventLoop* loop_;               // 当前线程所持有的 loop_
  std::string name_;              // 线程名称
  std::condition_variable cond_;  // 通知是否创建好 loop_
  std::thread thread_;            // 运行创建的 loop_
  bool exiting_;                  // 当前线程是否退出
  // 允许用户在 EventLoop, 线程启动后，事件循环开始前，执行一些初始化工作
  ThreadInitCallback callback_;
};

}  // namespace starry
