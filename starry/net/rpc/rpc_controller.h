// rpc_controller.h
#pragma once

#include <google/protobuf/service.h>
#include <atomic>
#include <string>

namespace starry {

class RpcController : public google::protobuf::RpcController {
 public:
  RpcController();

  // 实现 RpcController 接口
  void Reset() override;
  bool Failed() const override;
  std::string ErrorText() const override;
  void SetFailed(const std::string& reason) override;
  void StartCancel() override;
  bool IsCanceled() const override;
  void NotifyOnCancel(google::protobuf::Closure* callback) override;

  // 超时控制
  void SetTimeout(int32_t timeoutMs);
  int32_t GetTimeout() const;

 private:
  bool failed_;                                // 是否失败
  std::string errorText_;                      // 错误信息
  std::atomic<bool> canceled_;                 // 是否取消
  google::protobuf::Closure* cancelCallback_;  // 取消回调
  int32_t timeoutMs_;                          // 超时时间（毫秒）
};

}  // namespace starry
