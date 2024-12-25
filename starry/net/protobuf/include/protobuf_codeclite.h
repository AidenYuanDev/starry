#include <memory>
#include <string_view>

namespace google::protobuf {
class Message;
}

namespace starry {
  using MessagePtr = std::shared_ptr<google::protobuf::Message>;
  
  class ProtobufCodecLite {
    
  };
}
