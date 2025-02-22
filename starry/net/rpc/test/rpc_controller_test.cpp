#include <gtest/gtest.h>
#include "rpc_closure.h"
#include "rpc_controller.h"

namespace starry {

// 测试 RpcController 的基本功能
TEST(RpcControllerTest, BasicFunctionality) {
  RpcController controller;

  // 初始状态
  EXPECT_FALSE(controller.Failed());
  EXPECT_EQ(controller.ErrorText(), "");

  // 设置失败
  controller.SetFailed("Test error");
  EXPECT_TRUE(controller.Failed());
  EXPECT_EQ(controller.ErrorText(), "Test error");

  // 重置
  controller.Reset();
  EXPECT_FALSE(controller.Failed());
  EXPECT_EQ(controller.ErrorText(), "");
}

// 测试取消操作
TEST(RpcControllerTest, CancelOperation) {
  RpcController controller;

  // 初始状态
  EXPECT_FALSE(controller.IsCanceled());

  // 触发取消
  controller.StartCancel();
  EXPECT_TRUE(controller.IsCanceled());

  // 重置
  controller.Reset();
  EXPECT_FALSE(controller.IsCanceled());
}

// 测试取消通知
TEST(RpcControllerTest, CancelNotification) {
  RpcController controller;
  bool callbackInvoked = false;

  // 设置取消通知
  google::protobuf::Closure* callback =
      NewCallback([&callbackInvoked]() { callbackInvoked = true; });

  controller.NotifyOnCancel(callback);

  // 应该在取消时触发回调
  controller.StartCancel();
  EXPECT_TRUE(callbackInvoked);
}

}  // namespace starry

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
