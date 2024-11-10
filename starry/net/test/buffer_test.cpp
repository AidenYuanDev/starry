// buffer_test.cc
#include <gtest/gtest.h>
#include "buffer.h"
#include <string>

using namespace starry;

class BufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试前运行
        buffer_ = new Buffer();
    }

    void TearDown() override {
        // 在每个测试后运行
        delete buffer_;
    }

    Buffer* buffer_;
};

// 1. 基本操作测试
TEST_F(BufferTest, BasicOperations) {
    EXPECT_EQ(buffer_->readableBytes(), 0);
    EXPECT_EQ(buffer_->writableBytes(), Buffer::kInitialSize);
    EXPECT_EQ(buffer_->prependableBytes(), Buffer::kCheapPrepend);
    
    const std::string str(200, 'x');
    buffer_->append(str);
    EXPECT_EQ(buffer_->readableBytes(), str.size());
    EXPECT_EQ(buffer_->writableBytes(), Buffer::kInitialSize - str.size());
}

// 2. 读写操作测试
TEST_F(BufferTest, ReadWrite) {
    std::string str = "Hello, World!";
    buffer_->append(str);
    EXPECT_EQ(buffer_->readableBytes(), str.size());
    
    // 使用正确的比较方式
    EXPECT_EQ(std::string_view(buffer_->peek(), buffer_->readableBytes()), str);
    
    // 测试retrieve
    EXPECT_EQ(buffer_->retrieveAllAsString(), str);
    EXPECT_EQ(buffer_->readableBytes(), 0);
}

// 3. 搜索操作测试
TEST_F(BufferTest, SearchOperations) {
    std::string str = "Hello\r\nWorld\r\n";
    buffer_->append(str);
    
    const char* crlf = buffer_->findCRLF();
    EXPECT_TRUE(crlf != nullptr);
    EXPECT_EQ(crlf - buffer_->peek(), 5);  // "Hello" length
}

// 4. 空间管理测试
TEST_F(BufferTest, SpaceManagement) {
    // 4.1 空间扩展测试
    std::string str(1000, 'x');
    buffer_->append(str);
    EXPECT_GE(buffer_->writableBytes(), 0);
    
    // 4.2 内部腾挪测试
    buffer_->retrieve(900);
    buffer_->append(std::string(300, 'y'));
    EXPECT_EQ(buffer_->readableBytes(), 400);
}

// 5. Prepend操作测试
TEST_F(BufferTest, PrependOperations) {
    // 准备初始数据
    buffer_->append("world");
    
    // 测试prepend
    buffer_->prepend("hello ", 6);
    EXPECT_EQ(buffer_->retrieveAllAsString(), "hello world");
}

// 6. 网络操作测试
TEST_F(BufferTest, NetworkOperations) {
    // 创建测试用的pipe
    int pipefd[2];
    ASSERT_EQ(::pipe(pipefd), 0);
    
    // 写入数据到pipe
    std::string data = "Hello, Network!";
    ssize_t nw = ::write(pipefd[1], data.data(), data.size());
    EXPECT_EQ(nw, static_cast<ssize_t>(data.size()));
    
    // 从pipe读取数据到buffer
    ssize_t nr = buffer_->readFd(pipefd[0], nullptr);
    EXPECT_EQ(nr, static_cast<ssize_t>(data.size()));
    EXPECT_EQ(buffer_->retrieveAllAsString(), data);
    
    ::close(pipefd[0]);
    ::close(pipefd[1]);
}
