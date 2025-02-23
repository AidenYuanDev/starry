#pragma once

#include <memory>
#include "protobuf_codec_lite.h"

namespace starry {

// 前向声明
class Buffer;
class TcpConnection;
class RpcMessage;

// 类型别名定义
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using RpcMessagePtr = std::shared_ptr<RpcMessage>;

// RPC 消息格式:
//
// Field     Length  Content
//
// size      4-byte  N+8
// "RPC0"    4-byte
// payload   N-byte
// checksum  4-byte  adler32 of "RPC0"+payload

// RPC tag 定义
extern const char rpctag[];  // = "RPC0"

// RPC 编解码器类型定义
using RpcCodec = ProtobufCodecLiteT<RpcMessage, rpctag>;

}  // namespace starry
