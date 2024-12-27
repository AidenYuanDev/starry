#include "protobuf_codeclite.h"
#include "logging.h"
#include <tcp_connection.h>

using namespace starry;
namespace
{
  const std::string kNoErrorStr = "NoError";
  const std::string kInvalidLengthStr = "InvalidLength";
  const std::string kCheckSumErrorStr = "CheckSumError";
  const std::string kInvalidNameLenStr = "InvalidNameLen";
  const std::string kUnknownMessageTypeStr = "UnknownMessageType";
  const std::string kParseErrorStr = "ParseError";
  const std::string kUnknownErrorStr = "UnknownError";
}

const std::string& ProtobufCodecLite::errorCodeToString(ErrorCode errorCode)
{
  switch (errorCode)
  {
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
                                             ErrorCode errorCode)
{
  LOG_ERROR << "ProtobufCodecLite::defaultErrorCallback - " << errorCodeToString(errorCode);
  if (conn && conn->connected())
  {
    conn->shutdown();
  }
}
