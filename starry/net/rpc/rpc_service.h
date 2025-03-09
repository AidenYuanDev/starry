#include "rpcservice.pb.h"
#include "service.h"

namespace starry {

class RpcServiceImpl : public RpcService {
 public:
  RpcServiceImpl(const ServiceMap* services) : services_(services) {}

  virtual void listRpc(const ListRpcRequestPtr& request,
                       const ListRpcResponse* responsePrototype,
                       const RpcDoneCallback& done);

  virtual void getService(const GetServiceRequestPtr& request,
                          const GetServiceResponse* responsePrototype,
                          const RpcDoneCallback& done);

 private:
  const ServiceMap* services_;
};

}  // namespace starry
