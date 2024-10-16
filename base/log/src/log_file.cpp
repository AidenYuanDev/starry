#include <unistd.h>
#include <cassert>
#include <chrono>
#include <sstream>
#include "log_file.h"

namespace starry {

// 判断日志文件名是否合法
LogFile::LogFile(const std::string_view basename,
                 const std::string& directory,
                 size_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
    : basename_(basename),
      directory_(directory),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      checkEveryN_(checkEveryN),
      count_(0),
      mutex_(threadSafe ? std::make_unique<std::mutex>() : nullptr),
      lastRoll_(std::chrono::system_clock::now()),
      lastFlush_(std::chrono::system_clock::now()) {
  assert(!basename_.empty() && basename_ != "." && basename_ != "..");
  rollFile();
}

LogFile::~LogFile() = default;

// 追加日志:有锁和无锁
void LogFile::append(std::string_view logline) {
  if (mutex_) {
    std::lock_guard<std::mutex> lock(*mutex_);
    append_unlocked(logline);
  } else {
    append_unlocked(logline);
  }
}

// 刷新缓冲区：有锁和无锁
void LogFile::flush() {
  if (mutex_) {
    std::lock_guard<std::mutex> lock(*mutex_);
    file_ << std::flush;
  } else {
    file_ << std::flush;
  }
}

// 无锁追加日志
void LogFile::append_unlocked(std::string_view logline) {
  file_ << logline;
  if (file_.tellp() >
      static_cast<std::streampos>(rollSize_)) {  // 判断文件字节数
    rollFile();
  } else {
    ++count_;
    if (count_ >= checkEveryN_) {  // 判断写入频率
      count_ = 0;
      auto now = std::chrono::system_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::seconds>(now - lastRoll_)
              .count();
      if (duration > kRollPerSeconds_) {  // 判断文件存在的时间
        rollFile();
      } else if (now - lastFlush_ >
                 std::chrono::seconds(flushInterval_)) {  // 判断刷新间隔
        lastFlush_ = now;
        file_ << std::flush;
      }
    }
  }
}

// 滚动文件
bool LogFile::rollFile() {
  auto now = std::chrono::system_clock::now();
  std::string filename = getLogFileName(now);
  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    file_.close();
    file_.open(filename, std::ios_base::app);
    return true;
  }
  return false;
}

// 获取现在的日志名称 格式：文件名 + 日期 + 进程号 + 后缀
std::string LogFile::getLogFileName(
    const std::chrono::system_clock::time_point& now) {
  auto now_c = std::chrono::floor<std::chrono::seconds>(now);
  std::stringstream ss;
  ss << basename_ << '.' << std::format("{:%Y%m%d-%H:%M:%S}", now_c) << '.'
     << ::getpid() << ".log";
  return ss.str();
}

}  // namespace starry
