#include <endian.h>
#include <stdlib.h>
#include <cstddef>
#include "codec.h"

namespace starry {

RpcCodec::RpcCodec(const RpcErrorCallback& errorCb) : errorCallback_(errorCb) {}

void RpcCodec::encode(Buffer* buf, const rpc::core::RpcMessage& message) {
  // 序列化消息
  std::string data;
  if (!message.SerializeToString(&data)) {
    // 序列化失败处理
    rpc::core::RpcMessage errorMsg;
    errorMsg.set_type(rpc::core::RpcMessage::ERROR);
    errorMsg.set_error("Serialize message failed");
    errorCallback_(errorMsg);
    return;
  }

  // 写入长度(网络字节序)
  int32_t len = static_cast<int32_t>(data.size());
  buf->appendInt32(len);

  // 写入序列化数据
  buf->append(data);
}

void RpcCodec::decode(Buffer* buf) {
  while (buf->readableBytes() >= kHeaderLen) {
    // 检查消息完整性
    const int32_t len = buf->peekInt32();
    if (len < 0) {
      // 无效长度处理
      rpc::core::RpcMessage errorMsg;
      errorMsg.set_type(rpc::core::RpcMessage::ERROR);
      errorMsg.set_error("Invalid message length");
      errorCallback_(errorMsg);
      buf->retrieveAll();  // 清空buffer，防止继续解析错误数据
      return;
    }

    if (buf->readableBytes() >= static_cast<size_t>(kHeaderLen + len)) {
      rpc::core::RpcMessage message;
      if (parse(buf, &message)) {
        // 解析成功，调用回调
        messageCallback_(message);
      } else {
        // 解析失败，调用错误回调
        rpc::core::RpcMessage errorMsg;
        errorMsg.set_type(rpc::core::RpcMessage::ERROR);
        errorMsg.set_error("RPC message parse error");
        errorCallback_(errorMsg);
      }
    } else {
      // 数据不完整，等待更多数据
      break;
    }
  }
}

bool RpcCodec::parse(Buffer* buf, rpc::core::RpcMessage* message) {
  // 读取长度
  int32_t len = buf->readInt32();

  // 反序列化消息
  if (!message->ParseFromArray(buf->peek(), len)) {
    // 反序列化失败
    buf->retrieve(len);
    return false;
  }

  // 消息解析成功
  buf->retrieve(len);
  return true;
}

}  // namespace starry
