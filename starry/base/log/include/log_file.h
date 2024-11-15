#pragma once

#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

// 实现写入文件的逻辑
namespace starry {

class LogFile {
 public:
  LogFile(const std::string_view basename,
          size_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1024);

  ~LogFile();

  void append(std::string_view logline);  // 添加日志
  void flush();                           // 刷新缓冲区

 private:
  void append_unlocked(std::string_view logline);  // 不加锁的追加文件
  bool rollFile();                                 // 滚动文件
  std::string getLogFileName(const std::chrono::system_clock::time_point& now);

  const std::string basename_;                       // 文件名称
  size_t rollSize_;                                  // 单文件大小上限
  int flushInterval_;                                // 刷新间隔
  int checkEveryN_;                                  // 写入频率
  int count_;                                        // 实际写入频率
  std::unique_ptr<std::mutex> mutex_;                // 锁
  std::chrono::system_clock::time_point lastRoll_;   // 上次滚动时间
  std::chrono::system_clock::time_point lastFlush_;  // 上次刷新时间
  std::ofstream file_;                               // 打开的文件

  static const int kRollPerSeconds_ = 60 * 60 * 24;  // 24小时
};

}  // namespace starry
