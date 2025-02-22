// rpc_controller.cpp
#include "rpc_controller.h"

namespace starry {

RpcController::RpcController()
    : failed_(false),
      canceled_(false),
      cancelCallback_(nullptr),
      timeoutMs_(0) {}

void RpcController::Reset() {
  failed_ = false;
  errorText_.clear();
  canceled_ = false;
  cancelCallback_ = nullptr;
  timeoutMs_ = 0;
}

bool RpcController::Failed() const {
  return failed_;
}

std::string RpcController::ErrorText() const {
  return errorText_;
}

void RpcController::SetFailed(const std::string& reason) {
  failed_ = true;
  errorText_ = reason;
}

void RpcController::StartCancel() {
  canceled_ = true;
  if (cancelCallback_) {
    cancelCallback_->Run();
  }
}

bool RpcController::IsCanceled() const {
  return canceled_;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {
  cancelCallback_ = callback;
  if (canceled_ && callback) {
    callback->Run();
  }
}

void RpcController::SetTimeout(int32_t timeoutMs) {
  timeoutMs_ = timeoutMs;
}

int32_t RpcController::GetTimeout() const {
  return timeoutMs_;
}

}  // namespace starry
