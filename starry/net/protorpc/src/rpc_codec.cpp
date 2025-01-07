#include "rpc_codec.h"
#include <google/protobuf/stubs/common.h>

#include "google-inl.h"

using namespace starry;

namespace {
int ProtobufVersionCheck() {
  GOOGLE_PROTOBUF_VERSION;
  return 0;
}
int dummy __attribute__ ((unused)) = ProtobufVersionCheck();
}

namespace starry {
  const char rpctag [] = "RPC0"; 
}
