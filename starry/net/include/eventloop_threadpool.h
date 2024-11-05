#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace starry {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
  ~EventLoopThreadPool();

  // 设置线程数
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  // 开启循环
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  EventLoop* getNextLoop();                    // 获取一个 EventLoop
  EventLoop* getLoopForHash(size_t hashCode);  // 获取指定编号的 EventLoop
  std::vector<EventLoop*> getAllLoops();       // 获取所有的 EventLoop

  bool started() const { return started_; }  // 是否开启

 private:
  EventLoop* baseLoop_;  // 最顶层的 loop_ ,用来分发连接
  std::string name_;
  bool started_;         // 是否开启循环
  int numThreads_;       // 线程池线程数
  int next_;             // 下一个 EventLoop
  std::vector<std::unique_ptr<EventLoopThread>> threads_;  // 线程指针
  std::vector<EventLoop*> loops_;                          // EventLoop 集和
};

}  // namespace starry
