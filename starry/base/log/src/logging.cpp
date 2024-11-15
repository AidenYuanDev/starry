#include <bits/chrono.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <thread>
#include "logging.h"

namespace starry {

thread_local char t_errnobuf[512];

const char* strerror_tl(int saveErrno) {
  return strerror_r(saveErrno, t_errnobuf, sizeof(t_errnobuf));
}

// 初始化静态成员
LogLevel Logger::g_logLevel = LogLevel::INFO;
// 默认同步写入
Logger::OutputFunc Logger::g_output = [](const char* msg, int len) {
  std::cout.write(msg, len);
};
// 默认同步刷新
Logger::FlushFunc Logger::g_flush = [] { std::cout.flush(); };

Logger::Logger(LogLevel level, const std::source_location& loc)
    : level_(level), stream_(), location_(loc) {
  auto time_c = std::chrono::time_point_cast<std::chrono::seconds>(
      std::chrono::system_clock::now());
  stream_ << std::format("{:%F %T }", time_c);
  stream_ << "[" << std::this_thread::get_id() << "] " << levelToString(level_);
}

Logger::Logger(LogLevel level, int saveErrno, const std::source_location& loc)
    : Logger(level, loc) {
  if (saveErrno != 0) {
    stream_ << strerror_tl(saveErrno) << " (errno=" << saveErrno << ") ";
  }
}
Logger::~Logger() {
  finish();
  std::string_view buf = stream().data();
  g_output(buf.data(), buf.length());
  if (level_ == LogLevel::FATAL) {
    g_flush();
    abort();
  }
}

// 这条日志比g_logLevel高才可以输出,比ERROR的才可以刷新，超时刷新在async_logging
void Logger::finish() {
  std::string filename = location_.file_name();
  stream_ << " - " << filename.substr(filename.find_last_of('/') + 1) << ":" << location_.line() << "\n";
}

void Logger::setLogLevel(LogLevel level) {
  g_logLevel = level;
}

LogLevel Logger::logLevel() {
  return g_logLevel;
}

void Logger::setOutput(OutputFunc output) {
  g_output = std::move(output);
}

void Logger::setFlush(FlushFunc flush) {
  g_flush = std::move(flush);
}

std::string Logger::levelToString(LogLevel level) {
  switch (level) {
    case LogLevel::TRACE:
      return "TRACE ";
    case LogLevel::DEBUG:
      return "DEBUG ";
    case LogLevel::INFO:
      return "INFO ";
    case LogLevel::WARN:
      return "WARN ";
    case LogLevel::ERROR:
      return "ERROR ";
    case LogLevel::FATAL:
      return "FATAL ";
    default:
      return "UNKNOWN ";
  }
}

}  // namespace starry
