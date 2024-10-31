#pragma once

#include <endian.h>
#include <sys/types.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace starry {

class Buffer {
 public:
  static const size_t kCheapPreend = 8;     // 预留空间
  static const size_t kInitialSize = 1024;  // 初始化大小
  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPreend + initialSize),
        readerIndex_(kCheapPreend),
        writerIndex_(kCheapPreend) {}

  // 交换空间
  void swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  // 可读字节数
  size_t readableBytes() const { return writerIndex_ - readerIndex_; }
  // 可写字节数
  size_t writableBytes() const { return buffer_.size() - writerIndex_; }
  // 读指针前面的字节数
  size_t prependableBytes() const { return readerIndex_; }
  // 读指针
  const char* peek() const { return begin() + readerIndex_; }
  // 写指针
  char* beginWrite() { return begin() + writerIndex_; }

  // 查找换行符 \r\n
  const char* findCRLF() const {
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }
  // 查找换行符 \r\n
  const char* findCRLF(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }
  // 查找换行符 \n
  const char* findEOL() const {
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
  }
  // 查找换行符 \n
  const char* findEOL(const char* start) const {
    const void* eol =
        memchr(start, '\n', static_cast<size_t>(beginWrite() - start));
    return static_cast<const char*>(eol);
  }

  // 移动 readerIndex_ 指向的位置
  void retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else {
      retrieveAll();
    }
  }

  // 取出 [readerIndex_, end] 的字符
  void retrieveUntil(const char* end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(static_cast<size_t>(end - peek()));
  }
  // 取出 int64_t
  void retrieveInt64() { retrieve(sizeof(int64_t)); }
  // 取出 int32_t
  void retrieveInt32() { retrieve(sizeof(int32_t)); }
  // 取出 int16_t
  void retrieveInt16() { retrieve(sizeof(int16_t)); }
  // 取出 int8_t
  void retrieveInt8() { retrieve(sizeof(int8_t)); }
  // 取出所有字符
  std::string retrieveAllAsString() {
    return retrieveAsString(readableBytes());
  }
  // 取出 长度长度为 len 的字符
  std::string retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
  }

  // 添加长度为 len 的数据 data
  void append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
  }
  void append(const void* data, size_t len) {
    append(static_cast<const char*>(data), len);
  }
  void append(const std::string_view data) { append(data.data(), data.size()); }

  // 确保有空间写入，没有就开辟空间
  void ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
      makeSpace(len);
    }
    assert(writableBytes() >= len);
  }

  // 移动 writerIndex_
  void hasWritten(size_t len) {
    assert(len <= writableBytes());
    writerIndex_ += len;
  }

  // 移动 readerIndex_ writerIndex_
  void retrieveAll() {
    readerIndex_ = kCheapPreend;
    writerIndex_ = kCheapPreend;
  }

  // 写指针的位置
  const char* beginWrite() const { return begin() + writerIndex_; }

  // 回退长度为 len 的字符
  void unwrite(size_t len) {
    assert(len <= readableBytes());
    writerIndex_ -= len;
  }

  // 用作 读取和写入网络字节序
  // 写入, 网路字节序
  void appendInt64(int64_t x) {
    int64_t be64 = htobe64(x);
    append(&be64, sizeof(be64));
  }
  void appendInt32(int32_t x) {
    int32_t be32 = htobe32(x);
    append(&be32, sizeof(be32));
  }
  void appendInt16(int16_t x) {
    int16_t be16 = htobe16(x);
    append(&be16, sizeof(be16));
  }
  void appendInt8(int8_t x) { append(&x, sizeof(x)); }

  // 读取，不移动指针，网络字节序
  int64_t readInt64() {
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
  }
  int32_t readInt32() {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
  }
  int16_t readInt16() {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
  }
  int8_t readInt8() {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
  }
  int64_t peekInt64() const {
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return be64toh(be64);
  }
  int32_t peekInt32() const {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return be32toh(be32);
  }
  int16_t peekInt16() const {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return be16toh(be16);
  }
  int8_t peekInt8() const {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
  }

  // 读取，移动网络字节序
  void prependInt64(int64_t x) {
    int64_t be64 = htobe64(x);
    prepend(&be64, sizeof(be64));
  }

  void prependInt32(int32_t x) {
    int32_t be32 = htobe32(x);
    prepend(&be32, sizeof(be32));
  }

  void prependInt16(int16_t x) {
    int16_t be16 = htobe16(x);
    prepend(&be16, sizeof be16);
  }

  void prependInt8(int8_t x) { prepend(&x, sizeof x); }

  void prepend(const void* data, size_t len) {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + readerIndex_);
  }

  // 整理空间
  void shrink(size_t reserve) {
    Buffer other;
    other.ensureWritableBytes(readableBytes() + reserve);
    other.append(retrieveAllAsString());
    swap(other);
  }

  // 增加容量
  size_t internalCapacity() const { return buffer_.capacity(); }

  ssize_t readFd(int fd, int* savedErrno);

 private:
  char* begin() { return buffer_.data(); }              // buffer 的指针
  const char* begin() const { return buffer_.data(); }  // buffer 的指针

  // 增加空间
  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPreend) {
      buffer_.resize(writerIndex_ + len);
    } else {
      assert(kCheapPreend < readableBytes());
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_,
                begin() + kCheapPreend);
      readerIndex_ = kCheapPreend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }

 private:
  std::vector<char> buffer_;  // buffer
  size_t readerIndex_;        // 读指针
  size_t writerIndex_;        // 写指针

  static const char kCRLF[];  // /r/n
};

}  // namespace starry
