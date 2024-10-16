#include <bits/chrono.h>
#include <cmath>
#include <format>
#include <iostream>
#include <chrono>
#include "logging.h"

namespace starry {

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

Logger::~Logger() {
  finish();
}

// 这条日志比g_logLevel高才可以输出,比ERROR的才可以刷新，超时刷新在async_logging
void Logger::finish() {
  if (level_ >= g_logLevel) {
    stream_ << " - " << location_.file_name() << ":" << location_.line();
    auto time_c = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    std::string msg = std::format("{} {} {}\n", time_c,
                                  levelToString(level_), stream_.data());

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
