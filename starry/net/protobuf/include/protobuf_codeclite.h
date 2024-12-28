#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include "buffer.h"
#include "callbacks.h"

namespace google::protobuf {
class Message;
}

namespace starry {
using MessagePtr = std::shared_ptr<google::protobuf::Message>;

class ProtobufCodecLite {
 public:
  const static int kHeaderLen = sizeof(int32_t);
  const static int kChecksumLen = sizeof(int32_t);
  const static int kMaxMessageLen = 64 * 1024 * 1024;

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

  ProtobufCodecLite(const ::google::protobuf::Message* prototype,
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
            const ::google::protobuf::Message& message);

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

  virtual bool parseFromBuffer(std::string_view buf,
                               google::protobuf::Message* message);
  virtual int serializwToBuffer(const google::protobuf::Message& message,
                                Buffer* buf);

  static const std::string& errorCodeToString(ErrorCode errorCode);

  // 便于测试的函数
  ErrorCode parse(const char* buf,
                  int len,
                  ::google::protobuf::Message* message);
  void fillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message);

  static int32_t checksum(const void* buf, int len);
  static bool validateChecksum(const char* buf, int len);
  static int32_t asInt32(const char* buf);
  static void defaultErrorCallback(const TcpConnectionPtr&,
                                   Buffer*,
                                   Timestamp,
                                   ErrorCode);

 private:
  const ::google::protobuf::Message* prototype_;
  const std::string tag_;
  ProtobufMessageCallback messageCallback_;
  RawMessageCallback rawCb_;
  ErrorCallback errorCallback_;
  const int kMinMessageLen;
};

template <typename MSG, const char* TAG, typename CODEC = ProtobufCodecLite>
class ProtobufCodecLiteT {
  static_assert(std::is_base_of_v<ProtobufCodecLite, CODEC>,
                "CODEC should be derived from ProtobufCodecLite");

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

  void onRpcMessage(const TcpConnectionPtr& conn,
                    const MessagePtr& message,
                    Timestamp receiveTime) {
    messageCallback_(conn, down_pointer_case<MSG>(message), receiveTime);
  }

  void fillEmptyBuffer(Buffer* buf, const MSG& message) {
    codec_.fillEmptyBuffer(buf, message);
  }

 private:
  ProtobufMessageCallback messageCallback_;
  CODEC codec_;
};
}  // namespace starry
