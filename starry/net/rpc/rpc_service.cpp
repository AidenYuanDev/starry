#include "rpc_service.h"

#include <google/protobuf/descriptor.h>
#include "rpc.pb.h"
#include "rpcservice.pb.h"
#include "service.h"

using namespace starry;

void addMethodNames(const ::google::protobuf::ServiceDescriptor* sd,
                    ListRpcResponse* response) {
  for (int i = 0; i < sd->method_count(); i++) {
    const ::google::protobuf::MethodDescriptor* method = sd->method(i);
    std::string methodName;
    methodName.reserve(10
                       + method->full_name().size()
                       + method->input_type()->full_name().size()
                       + method->output_type()->full_name().size());
    methodName.append(method->full_name());
    methodName.append(" (");
    methodName.append(method->input_type()->full_name());
    methodName.append(") : ");
    methodName.append(method->output_type()->full_name());
    response->add_method_name(methodName);
  }
}

void RpcServiceImpl::listRpc(const ListRpcRequestPtr& request,
                             const ListRpcResponse* responsePrototype,
                             const RpcDoneCallback& done) {
  ListRpcResponse response;
  if (!request->service_name().empty()) {
    ServiceMap::const_iterator it = services_->find(request->service_name());
    if (it != services_->end()) {
      response.set_error(NO_ERROR);
      response.add_method_name(it->first);
      if (request->list_method()) {
        addMethodNames(it->second->GetDescriptor(), &response);
      }
    } else {
      response.set_error(NO_METHOD);
    }
  } else {
    response.set_error(NO_ERROR);
    for (ServiceMap::const_iterator it = services_->begin();
         it != services_->end(); it++) {
      // response.add_service_name(it->first)
    }
  }

}
