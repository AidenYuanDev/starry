#include <gtest/gtest.h>
#include <memory>
#include "buffer.h"
#include "codec.h"
#include "rpc_message.pb.h"

namespace starry {

// 测试夹具
class RpcCodecTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // 创建带错误回调的编解码器
    codec_ = std::make_unique<RpcCodec>(
        [this](const rpc::core::RpcMessage& msg) { errorCount_++; });
  }

  std::unique_ptr<RpcCodec> codec_;
  int errorCount_ = 0;
};

// 测试基本的编解码功能
TEST_F(RpcCodecTest, BasicEncodeDecode) {
  // 创建测试消息
  rpc::core::RpcMessage message;
  message.set_type(rpc::core::RpcMessage::REQUEST);
  message.set_id(42);
  message.set_service("TestService");
  message.set_method("TestMethod");
  message.set_payload("test payload");

  // 编码消息
  Buffer buf;
  codec_->encode(&buf, message);

  // 设置解码回调
  rpc::core::RpcMessage decoded;
  bool callbackInvoked = false;

  codec_->setMessageCallback(
      [&decoded, &callbackInvoked](const rpc::core::RpcMessage& msg) {
        decoded = msg;
        callbackInvoked = true;
      });

  // 解码消息
  codec_->decode(&buf);

  // 验证结果
  EXPECT_TRUE(callbackInvoked);
  EXPECT_EQ(decoded.type(), message.type());
  EXPECT_EQ(decoded.id(), message.id());
  EXPECT_EQ(decoded.service(), message.service());
  EXPECT_EQ(decoded.method(), message.method());
  EXPECT_EQ(decoded.payload(), message.payload());
}

// 测试错误处理
TEST_F(RpcCodecTest, ErrorHandling) {
  // 创建无效数据
  Buffer buf;
  buf.appendInt32(-1);  // 负长度，这应该触发错误处理

  // 设置一个空回调，我们只关心错误处理
  codec_->setMessageCallback([](const rpc::core::RpcMessage&) {});

  // 尝试解码
  codec_->decode(&buf);

  // 验证错误处理被调用
  EXPECT_EQ(errorCount_, 1);
}

}  // namespace starry

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
