#include "protobuf_codec_lite.h"

#include <google/protobuf/arena.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <zconf.h>
#include <zlib.h>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "buffer.h"
#include "callbacks.h"
#include "endian.h"
#include "logging.h"
#include "tcp_connection.h"
#include "types.h"

namespace starry {

void ProtobufCodecLite::send(const TcpConnectionPtr& conn,
                             const google::protobuf::Message& message) {
  google::protobuf::Arena arena;
  Buffer buf;
  fillEmptyBuffer(&buf, message);
  conn->send(&buf);
}

void ProtobufCodecLite::fillEmptyBuffer(
    Buffer* buf,
    const google::protobuf::Message& message) {
  assert(buf->readableBytes() == 0);

  // 预留足够的空间，避免多次内存分配
  // size_t messageSize = message.ByteSizeLong();
  // size_t totalSize = kHeaderLen + tag_.size() + messageSize + kChecksumLen;
  // buf->ensureWritableBytes(totalSize);

  // 添加 tag
  buf->append(tag_);

  // 优化序列化过程
  int byte_size = serializeToBuffer(message, buf);

  // 计算和添加校验和
  int32_t checkSum = checksum(std::span(buf->peek(), buf->readableBytes()));
  buf->appendInt32(checkSum);

  assert(buf->readableBytes() == tag_.size() + byte_size + kChecksumLen);

  // 添加总长度
  int32_t len = htobe32(static_cast<int32_t>(buf->readableBytes()));
  buf->prepend(&len, sizeof len);
}

bool ProtobufCodecLite::parseFromBuffer(std::span<const char> buf,
                                        google::protobuf::Message* message) {
  return message->ParseFromArray(buf.data(), buf.size());
}

int ProtobufCodecLite::serializeToBuffer(
    const google::protobuf::Message& message,
    Buffer* buf) {
  // 检查消息是否已初始化
  if (!message.IsInitialized()) {
    LOG_ERROR
        << "Failed to serialize message because it is missing required filds.";
    return -1;
  }

  size_t byte_size = message.ByteSizeLong();
  buf->ensureWritableBytes(byte_size);

  uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
  uint8_t* end = message.SerializeWithCachedSizesToArray(start);

  if (end - start != static_cast<uint8_t>(byte_size)) {
    LOG_ERROR << "Failed to serialize message, size mismatch";
    return -1;
  }

  buf->hasWritten(byte_size);
  return byte_size;
}

void ProtobufCodecLite::onMessage(const TcpConnectionPtr& conn,
                                  Buffer* buf,
                                  Timestamp receiveTime) {
  // 使用 Arena 优化内存分配
  // google::protobuf::Arena arena;

  while (buf->readableBytes() >= static_cast<uint32_t>(kMinMessageLen + kHeaderLen)) {
    const int32_t len = buf->peekInt32();
    if (len > kMaxMessageLen || len < kMinMessageLen) {
      errorCallback_(conn, buf, receiveTime, ErrorCode::kInvalidLength);
      break;
      ;
    }

    if (buf->readableBytes() >= implicit_cast<size_t>(kHeaderLen + len)) {
      // 不使用Arena分配消息对象
      MessagePtr message(prototype_->New());  // 移除arena参数

      ErrorCode errorCode =
          parse(std::span(buf->peek() + kHeaderLen, len), message.get());

      if (errorCode == ErrorCode::kNoError) {
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(kHeaderLen + len);
      } else {
        errorCallback_(conn, buf, receiveTime, errorCode);
        break;
        ;
      }
    } else {
      break;
    }
  }
}

int32_t ProtobufCodecLite::checksum(std::span<const char> buf) {
  // 使用 SSE/AVX 优化的 Adler32 实现
  return static_cast<int32_t>(
      ::adler32_z(1, reinterpret_cast<const Byte*>(buf.data()), buf.size()));
}

namespace {
const std::string kNoErrorStr = "NoError";
const std::string kInvalidLengthStr = "InvalidLength";
const std::string kCheckSumErrorStr = "CheckSumError";
const std::string kInvalidNameLenStr = "InvalidNameLen";
const std::string kUnknownMessageTypeStr = "UnknownMessageType";
const std::string kParseErrorStr = "ParseError";
const std::string kUnknownErrorStr = "UnknownError";
}  // namespace

const std::string& ProtobufCodecLite::errorCodeToString(ErrorCode errorCode) {
  switch (errorCode) {
    case ErrorCode::kNoError:
      return kNoErrorStr;
    case ErrorCode::kInvalidLength:
      return kInvalidLengthStr;
    case ErrorCode::kCheckSumError:
      return kCheckSumErrorStr;
    case ErrorCode::kInvalidNameLen:
      return kInvalidNameLenStr;
    case ErrorCode::kUnknownMessageType:
      return kUnknownMessageTypeStr;
    case ErrorCode::kParseError:
      return kParseErrorStr;
    default:
      return kUnknownErrorStr;
  }
}

void ProtobufCodecLite::defaultErrorCallback(const TcpConnectionPtr& conn,
                                             Buffer*,
                                             Timestamp,
                                             ErrorCode errorCode) {
  LOG_ERROR << "ProtobufCodecLite::defaultErrorCallback - "
            << errorCodeToString(errorCode);
  if (conn && conn->connected()) {
    conn->shutdown();
  }
}

bool ProtobufCodecLite::validateChecksum(std::span<const char> buf) {
  // 校验和在末尾的 4 字节
  int32_t expectedCheckSum = asInt32(buf.data() + buf.size() - kChecksumLen);
  // 计算除校验和外的数据的校验和
  int32_t checkSum = checksum(buf.subspan(0, buf.size() - kChecksumLen));
  return checkSum == expectedCheckSum;
}

int32_t ProtobufCodecLite::asInt32(const char* buf) {
  int32_t be32 = 0;
  ::memcpy(&be32, buf, sizeof(be32));
  return htobe32(be32);
}

ProtobufCodecLite::ErrorCode ProtobufCodecLite::parse(
    std::span<const char> buf,
    google::protobuf::Message* message) {
  ErrorCode error = ErrorCode::kNoError;

  if (validateChecksum(buf)) {
    // 检查消息标签
    if (memcmp(buf.data(), tag_.data(), tag_.size()) == 0) {
      // 解析消息体
      const char* data = buf.data() + tag_.size();
      int32_t dataLen = buf.size() - kChecksumLen - tag_.size();
      if (parseFromBuffer(std::span(data, dataLen), message)) {
        error = ErrorCode::kNoError;
      } else {
        error = ErrorCode::kParseError;
      }
    } else {
      error = ErrorCode::kUnknownMessageType;
    }
  } else {
    error = ErrorCode::kCheckSumError;
  }

  return error;
}

}  // namespace starry
