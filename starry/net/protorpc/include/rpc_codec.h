#pragma once

#include <memory>
#include "callbacks.h"
#include "protobuf_codeclite.h"
#include "tcp_connection.h"

namespace starry {

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class RpcMessage;
using RpcMessagePtr = std::shared_ptr<RpcMessage>;
extern const char rpctag[];

using RpcCodec = ProtobufCodecLiteT<RpcMessage, rpctag>;
}
