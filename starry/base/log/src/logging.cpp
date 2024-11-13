#include <bits/chrono.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <format>
#include <iostream>
#include <source_location>
#include <string>
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
// 默认异步写入
Logger::FlushFunc Logger::g_flush = [] { std::cout.flush(); };

Logger::Logger(LogLevel level, const std::source_location& loc)
    : level_(level), stream_(), location_(loc) {}

Logger::Logger(LogLevel level, int saveErrno, const std::source_location& loc)
    : level_(level), stream_(), location_(loc) {
  stream_ << std::this_thread::get_id();
  stream_ << levelToString(level);
  if (saveErrno != 0) {
    stream_ << strerror_tl(saveErrno) << " (errno=" << saveErrno << ") ";
  }
}
Logger::~Logger() {
  finish();
}

// 这条日志比g_logLevel高才可以输出,比ERROR的才可以刷新，超时刷新在async_logging
void Logger::finish() {
  if (level_ >= g_logLevel) {
    std::string filename = location_.file_name();
    size_t last_slash = filename.find_last_of("/\\");
    if (last_slash != std::string::npos) {
      filename = filename.substr(last_slash + 1);
    }
    auto time_c = std::chrono::system_clock::now();
    std::string msg =
        std::format(" - {}:{} {:%F %T} {} {}\n", filename, location_.line(),
                    time_c, levelToString(level_), stream_.data());

    if (g_output) {
      g_output(msg.c_str(), static_cast<int>(msg.length()));
    }

    if (level_ >= LogLevel::ERROR && g_flush) {
      g_flush();
    }

    if (level_ == LogLevel::FATAL) {
      g_flush();
      std::abort();
    }
  }
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
      return "TRACE";
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO ";
    case LogLevel::WARN:
      return "WARN ";
    case LogLevel::ERROR:
      return "ERROR";
    case LogLevel::FATAL:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

}  // namespace starry
