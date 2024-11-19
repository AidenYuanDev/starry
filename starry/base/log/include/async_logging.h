#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "log_stream.h"

// 异步日志
namespace starry {

class AsyncLogging {
 public:
  AsyncLogging(const std::string& basename,
               off_t rollSize,
               int flushInterval = 3);
  ~AsyncLogging();

  void append(const char* logline, int len);  // 追加日志

  void start();  // 开始
  void stop();   // 结束

 private:
  void threadFunc();  // 开始异步

  using Buffer = LogStream<kLargeBuffer>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferVector = std::vector<BufferPtr>;

  const int flushInterval_;       // 刷新间隔
  std::atomic<bool> running_;     // 运行
  const std::string basename_;    // 文件名称
  const std::string directory_;   // 文件目录
  const off_t rollSize_;          // 文件大小
  std::thread thread_;            // 线程
  std::mutex mutex_;              // 锁
  std::condition_variable cond_;  // 条件变量
  BufferPtr currentBuffer_;       // 当前缓冲区
  BufferPtr nextBuffer_;          // 下一个缓冲区
  BufferVector buffers_;          // 存储缓冲区
};

}  // namespace starry
