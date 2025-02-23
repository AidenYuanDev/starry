#pragma once

#include <google/protobuf/message.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include "buffer.h"
#include "callbacks.h"
#include "tcp_connection.h"

namespace google::protobuf {
class Message;
}

namespace starry {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

class ProtobufCodecLite {
 public:
  static constexpr int kHeaderLen = sizeof(int32_t);
  static constexpr int kChecksumLen = sizeof(int32_t);
  static constexpr int kMaxMessageLen = 64 * 1024 * 1024;

  enum class ErrorCode {
    kNoError = 0,
    kInvalidLength,
    kCheckSumError,
    kInvalidNameLen,
    kUnknownMessageType,
    kParseError,
  };

  using RawMessageCallback =
      std::function<bool(const TcpConnectionPtr&, std::string_view, Timestamp)>;

  using ProtobufMessageCallback = std::function<
      void(const TcpConnectionPtr&, const MessagePtr&, Timestamp)>;

  using ErrorCallback = std::function<
      void(const TcpConnectionPtr&, Buffer*, Timestamp, ErrorCode)>;

  // 构造函数
  ProtobufCodecLite(const google::protobuf::Message* prototype,
                    std::string_view tagArg,
                    const ProtobufMessageCallback& messageCb,
                    const RawMessageCallback& rawCb = RawMessageCallback(),
                    const ErrorCallback& errorCb = defaultErrorCallback)
      : prototype_(prototype),
        tag_(tagArg),
        messageCallback_(messageCb),
        rawCb_(rawCb),
        errorCallback_(errorCb),
        kMinMessageLen(tagArg.size() + kChecksumLen) {}

  virtual ~ProtobufCodecLite() = default;

  const std::string& tag() const { return tag_; }

  void send(const TcpConnectionPtr& conn,
            const google::protobuf::Message& message);

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

  virtual bool parseFromBuffer(std::span<const char> buf,
                               google::protobuf::Message* message);

  virtual int serializeToBuffer(const google::protobuf::Message& message,
                                Buffer* buf);

  static const std::string& errorCodeToString(ErrorCode errorCode);
  static void defaultErrorCallback(const TcpConnectionPtr&,
                                   Buffer*,
                                   Timestamp,
                                   ErrorCode);

  ErrorCode parse(std::span<const char> buf,
                  google::protobuf::Message* message);
  void fillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message);

  static int32_t checksum(std::span<const char> buf);
  static bool validateChecksum(std::span<const char> buf);
  static int32_t asInt32(const char* buf);

 private:
  const google::protobuf::Message* prototype_;
  const std::string tag_;
  ProtobufMessageCallback messageCallback_;
  RawMessageCallback rawCb_;
  ErrorCallback errorCallback_;
  const int kMinMessageLen;
};

template <typename MSG, const char* TAG>
class ProtobufCodecLiteT {
  static_assert(std::is_base_of_v<google::protobuf::Message, MSG>,
                "MSG must be derived from google::protobuf::Message");

 public:
  using ConcreteMessagePtr = std::shared_ptr<MSG>;
  using ProtobufMessageCallback = std::function<
      void(const TcpConnectionPtr&, const ConcreteMessagePtr&, Timestamp)>;
  using RawMessageCallback = ProtobufCodecLite::RawMessageCallback;
  using ErrorCallback = ProtobufCodecLite::ErrorCallback;

  explicit ProtobufCodecLiteT(
      const ProtobufMessageCallback& messageCb,
      const RawMessageCallback& rawCb = RawMessageCallback(),
      const ErrorCallback& errorCb = ProtobufCodecLite::defaultErrorCallback)
      : messageCallback_(messageCb),
        codec_(&MSG::default_instance(),
               TAG,
               std::bind(&ProtobufCodecLiteT::onRpcMessage,
                         this,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3),
               rawCb,
               errorCb) {}

  const std::string& tag() const { return codec_.tag(); }

  void send(const TcpConnectionPtr& conn, const MSG& message) {
    codec_.send(conn, message);
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime) {
    codec_.onMessage(conn, buf, receiveTime);
  }

  // 为了单元测试
  void fillEmptyBuffer(Buffer* buf, const MSG& message) {
    codec_.fillEmptyBuffer(buf, message);
  }

 private:
  void onRpcMessage(const TcpConnectionPtr& conn,
                    const MessagePtr& message,
                    Timestamp receiveTime) {
    messageCallback_(conn, std::static_pointer_cast<MSG>(message), receiveTime);
  }

  ProtobufMessageCallback messageCallback_;
  ProtobufCodecLite codec_;
};

}  // namespace starry
