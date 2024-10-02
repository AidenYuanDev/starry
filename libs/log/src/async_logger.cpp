#include <chrono>
#include "async_logging.h"
#include "log_file.h"

namespace starry {

AsyncLogging::AsyncLogging(const std::string& basename,
                           const std::string& directory,
                           off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(basename),
      directory_(directory),
      rollSize_(rollSize),
      thread_(),
      currentBuffer_(std::make_unique<Buffer>()),
      nextBuffer_(std::make_unique<Buffer>()),
      buffers_() {
  currentBuffer_->clear();
  nextBuffer_->clear();
}

AsyncLogging::~AsyncLogging() {
  if (running_) {
    stop();
  }
}

// 核心：如果当前缓冲区有空间就追加到当前缓冲区，否则就存储当前缓冲区，并移动备用缓冲区作为当前缓冲区（减少开销）
void AsyncLogging::append(const char* logline, int len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(std::string_view(logline, len));
  } else {
    buffers_.push_back(std::move(currentBuffer_));

    if (nextBuffer_) {
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      currentBuffer_ = std::make_unique<Buffer>();
    }

    currentBuffer_->append(std::string_view(logline, len));
    cond_.notify_one();
  }
}

// 处理存储的缓冲区，把所有的缓冲区全部写入，释放空间剩余两个，移动到当前和备用缓冲区
void AsyncLogging::threadFunc() {
  LogFile output(basename_, directory_, rollSize_, false);
  BufferPtr newBuffer1 = std::make_unique<Buffer>();
  BufferPtr newBuffer2 = std::make_unique<Buffer>();
  newBuffer1->clear();
  newBuffer2->clear();
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);

  while (running_) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty()) {
        cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    if (buffersToWrite.size() > 25) {
      // 丢弃多余的日志，保留最旧的两个和最新的一个
      buffersToWrite.erase(buffersToWrite.begin() + 2,
                           buffersToWrite.end() - 1);
    }

    for (const auto& buffer : buffersToWrite) {
      output.append(buffer->data());
    }

    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->clear();
    }

    if (!newBuffer2) {
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->clear();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}

// 创建异步处理的线程
void AsyncLogging::start() {
  running_ = true;
  thread_ = std::thread(&AsyncLogging::threadFunc, this);
}

void AsyncLogging::stop() {
  running_ = false;
  cond_.notify_one();
  thread_.join();
}

}  // namespace starry
