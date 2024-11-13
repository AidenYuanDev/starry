#pragma once

#include <functional>
#include <source_location>
#include "log_stream.h"

// 用户使用的接口
namespace starry {

// 设置日志级别
enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

class Logger {
 public:
  using OutputFunc = std::function<void(const char* msg, int len)>;
  using FlushFunc = std::function<void()>;

  Logger(LogLevel level,
         const std::source_location& loc = std::source_location::current());

  Logger(LogLevel level,
         int savedErrno,
         const std::source_location& loc = std::source_location::current());
  ~Logger();

  SmallLogStream& stream() { return stream_; }  // 返回当前缓冲区

  static void setLogLevel(LogLevel level);  // 设置日志级别
  static LogLevel logLevel();               // 返回当前全局日志级别

  static void setOutput(OutputFunc output);  // 设置自定义输出逻辑
  static void setFlush(FlushFunc flush);     // 设置写入逻辑

 private:
  void finish();                                     // 写入日志
  static std::string levelToString(LogLevel level);  // 日志级别转字符串

  LogLevel level_;                 // 日志级别
  SmallLogStream stream_;          // 缓冲区
  std::source_location location_;  // 文件位置

  static LogLevel g_logLevel;  // 全局日志级别
  static OutputFunc g_output;  // 用户自定义输出
  static FlushFunc g_flush;    // 用户自定义写入
};

// 辅助宏，用于简化日志使用
#define LOG_TRACE                                            \
  if (starry::Logger::logLevel() <= starry::LogLevel::TRACE) \
  starry::Logger(starry::LogLevel::TRACE).stream()
#define LOG_DEBUG                                            \
  if (starry::Logger::logLevel() <= starry::LogLevel::DEBUG) \
  starry::Logger(starry::LogLevel::DEBUG).stream()
#define LOG_INFO                                            \
  if (starry::Logger::logLevel() <= starry::LogLevel::INFO) \
  starry::Logger(starry::LogLevel::INFO).stream()
#define LOG_WARN starry::Logger(starry::LogLevel::WARN).stream()
#define LOG_ERROR starry::Logger(starry::LogLevel::ERROR).stream()
#define LOG_FATAL starry::Logger(starry::LogLevel::FATAL).stream()
#define LOG_SYSERR starry::Logger(starry::LogLevel::ERROR, errno).stream()
#define LOG_SYSFATAL starry::Logger(starry::LogLevel::FATAL, errno).stream()
// 错误处理

const char* strerror_tl(int savedErrno);

}  // namespace starry
