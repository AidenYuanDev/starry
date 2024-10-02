#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include "log_stream.h"
#include "logging.h"
#include "async_logging.h"

namespace fs = std::filesystem;
using namespace starry;

// 辅助函数：生成指定大小的随机字符串
std::string generateRandomString(size_t length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
}

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时目录用于测试
        test_dir_ = fs::temp_directory_path() / "logger_test";
        fs::create_directories(test_dir_);
    }

    void TearDown() override {
        // 清理测试目录
        fs::remove_all(test_dir_);
    }

    fs::path test_dir_;
};

// LogStream 测试
TEST_F(LoggerTest, LogStreamBasicOperations) {
    starry::LogStream<starry::kLargeBuffer> stream;
    stream << "Hello" << 42 << 3.14;
    EXPECT_EQ(stream.data(), "Hello423.140000");

    stream.clear();
    EXPECT_TRUE(stream.data().empty());
}

// Logger 测试
TEST_F(LoggerTest, LoggerLevelAndBasicLogging) {
    starry::Logger::setLogLevel(starry::LogLevel::INFO);
    EXPECT_EQ(starry::Logger::logLevel(), starry::LogLevel::INFO);

    // 捕获日志输出
    std::string capturedLog;
    starry::Logger::setOutput([&capturedLog](const char* msg, int len) {
        capturedLog.append(msg, len);
    });

    LOG_INFO << "Test info message";
    EXPECT_TRUE(capturedLog.find("Test info message") != std::string::npos);

    capturedLog.clear();
    LOG_DEBUG << "Test debug message";  // 不应该被记录
    EXPECT_TRUE(capturedLog.empty());
}

TEST_F(LoggerTest, LoggingPerformance) {
    const int NUM_LOGS = 100000;
    const std::string LOG_FILENAME = "performance_test.log";
    
    // 使用 AsyncLogging 来处理高频率日志
    starry::AsyncLogging asyncLog(LOG_FILENAME, test_dir_.string(), 64 * 1024);
    asyncLog.start();

    // 设置 Logger 的输出函数
    starry::Logger::setOutput([&](const char* msg, int len) {
        asyncLog.append(msg, len);
    });

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_LOGS; ++i) {
        LOG_INFO << "Performance test log message " << i;
        if (i % 10000 == 0) {
            std::cout << "Logged " << i << " messages" << std::endl;
        }
    }

    // 确保所有日志都被写入
    asyncLog.stop();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time to log " << NUM_LOGS << " messages: " 
              << duration.count() << " ms" << std::endl;

    EXPECT_LT(duration.count(), 5000);  // 假设期望小于5秒
}
