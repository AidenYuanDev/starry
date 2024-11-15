#include <fcntl.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <regex>
#include "log_stream.h"
#include "logging.h"

namespace fs = std::filesystem;
using namespace starry;

// 辅助函数：字符串转日志级别
LogLevel stringToLogLevel(const std::string& level) {
  if (level == "TRACE")
    return LogLevel::TRACE;
  if (level == "DEBUG")
    return LogLevel::DEBUG;
  if (level == "INFO")
    return LogLevel::INFO;
  if (level == "WARN")
    return LogLevel::WARN;
  if (level == "ERROR")
    return LogLevel::ERROR;
  if (level == "FATAL")
    return LogLevel::FATAL;
  return LogLevel::INFO;  // 默认
}

class LoggerTest : public ::testing::Test {
 protected:
  std::string captured_log;
  fs::path test_dir_;

  void SetUp() override {
    // 设置测试用的输出函数
    Logger::setOutput(
        [this](const char* msg, int len) { captured_log.append(msg, len); });

    Logger::setFlush([] {
      // 测试时不需要实际刷新
    });

    captured_log.clear();

    // 创建临时目录用于测试
    test_dir_ = fs::temp_directory_path() / "logger_test";
    fs::create_directories(test_dir_);
  }

  void TearDown() override {
    // 重置为默认输出
    Logger::setOutput(
        [](const char* msg, int len) { std::cout.write(msg, len); });
    Logger::setFlush([] { std::cout.flush(); });

    // 清理测试目录
    fs::remove_all(test_dir_);
  }

  bool LogContains(const std::string& substr) {
    return captured_log.find(substr) != std::string::npos;
  }
};

// LogStream 基础测试
TEST_F(LoggerTest, LogStreamBasicOperations) {
  LogStream<kLargeBuffer> stream;
  stream << "Hello" << 42 << 3.14;
  EXPECT_EQ(stream.data(), "Hello423.14");  // 修改为实际输出格式

  stream.clear();
  EXPECT_TRUE(stream.data().empty());
}

// LogStream 指针测试
TEST_F(LoggerTest, LogStreamPointerHandling) {
  LogStream<kSmallBuffer> stream;
  int x = 42;
  int* ptr = &x;
  stream << ptr;
  EXPECT_FALSE(stream.data().empty());
  EXPECT_NE(stream.data().find("0x"), std::string::npos);

  stream.clear();
  ptr = nullptr;
  stream << ptr;
  EXPECT_EQ(stream.data(), "(null)");
}

// Logger 日志级别测试
TEST_F(LoggerTest, LoggerLevelAndBasicLogging) {
  Logger::setLogLevel(LogLevel::INFO);
  EXPECT_EQ(Logger::logLevel(), LogLevel::INFO);

  LOG_INFO << "Test info message";
  EXPECT_TRUE(LogContains("Test info message"));

  captured_log.clear();
  LOG_DEBUG << "Test debug message";  // 不应该被记录
  EXPECT_TRUE(captured_log.empty());
}

// 系统错误日志测试
TEST_F(LoggerTest, SystemErrorLogging) {
  LOG_ERROR << "Normal error message";
  std::string normal_error = captured_log;
  captured_log.clear();

  errno = ENOENT;
  LOG_SYSERR << "System error message";

  EXPECT_FALSE(normal_error.find("errno=") != std::string::npos);
  EXPECT_TRUE(LogContains("errno=2"));  // 只检查错误码
  EXPECT_TRUE(LogContains("System error message"));
}

// 实际文件操作场景测试
TEST_F(LoggerTest, FileOperationLogging) {
  fs::path test_file = test_dir_ / "nonexistent.txt";
  int fd = open(test_file.c_str(), O_RDONLY);
  if (fd < 0) {
    LOG_SYSERR << "Failed to open file " << test_file;
  }
  EXPECT_TRUE(LogContains("Failed to open file"));
  EXPECT_TRUE(LogContains("errno=2"));
}

// 日志级别过滤测试
TEST_F(LoggerTest, LogLevelFiltering) {
  LogLevel original_level = Logger::logLevel();

  Logger::setLogLevel(LogLevel::ERROR);
  LOG_INFO << "Should not appear";
  EXPECT_TRUE(captured_log.empty());

  LOG_ERROR << "Should appear";
  EXPECT_TRUE(LogContains("Should appear"));

  captured_log.clear();
  Logger::setLogLevel(LogLevel::TRACE);
  LOG_INFO << "Should now appear";
  EXPECT_TRUE(LogContains("Should now appear"));

  Logger::setLogLevel(original_level);
}

// 源码位置信息测试
TEST_F(LoggerTest, SourceLocationInfo) {
  captured_log.clear();

  // 获取当前行号
  int current_line = __LINE__;
  LOG_INFO << "Test message";

  // Debug: 输出实际的日志内容
  std::cout << "Captured log: [" << captured_log << "]\n";

  // 获取当前文件名（不包含路径）
  std::string filename = __FILE__;
  filename = filename.substr(filename.find_last_of("/\\") + 1);

  // 检查各个组成部分
  std::regex time_re(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");  // 时间戳格式
  std::regex thread_id_re(R"(\[\w+\])");  // 线程ID格式 [12345]
  std::regex location_re(filename + ":" +
                         std::to_string(current_line + 1));  // 文件位置

  // 逐个验证日志的各个部分
  EXPECT_TRUE(std::regex_search(captured_log, time_re))
      << "Log should contain timestamp";

  EXPECT_TRUE(std::regex_search(captured_log, thread_id_re))
      << "Log should contain thread id";

  EXPECT_TRUE(std::regex_search(captured_log, location_re))
      << "Log should contain correct file location";

  EXPECT_TRUE(LogContains("INFO")) << "Log should contain level";

  EXPECT_TRUE(LogContains("Test message")) << "Log should contain message";
}

// 日志格式综合测试
TEST_F(LoggerTest, LogFormatting) {
  captured_log.clear();

  errno = EACCES;  // Permission denied
  LOG_SYSERR << "Test error message";

  std::cout << "System error log: [" << captured_log << "]\n";

  // 修改正则表达式以匹配实际格式
  std::regex log_format_re(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})"  // 时间戳
                           R"( \[\d+\] )"            // 线程ID [数字]
                           R"(ERROR )"               // 日志级别
                           R"(.*\(errno=13\))"       // 错误信息
                           R"( Test error message)"  // 用户消息
                           R"( - .*:\d+\n)"          // 文件名:行号
  );

  EXPECT_TRUE(std::regex_search(captured_log, log_format_re))
      << "Log format should match expected pattern\n"
      << "Actual log: " << captured_log;
}

// 多日志级别的格式测试
TEST_F(LoggerTest, MultiLevelFormatting) {
  captured_log.clear();

  LOG_INFO << "Info message";
  LOG_WARN << "Warning message";
  LOG_ERROR << "Error message";

  std::string log_content = captured_log;
  std::cout << "Multi-level log content: [" << log_content << "]\n";

  // 检查每个级别
  std::vector<std::pair<std::string, std::string>> levels = {
      {"INFO", "Info message"},
      {"WARN", "Warning message"},
      {"ERROR", "Error message"}};

  for (const auto& [level, message] : levels) {
    // 构建完整的正则表达式字符串
    std::string pattern = R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})"  // 时间戳
                          R"( \[\d+\] )" +  // 线程ID [数字]
                          level +
                          " " +              // 日志级别
                          message +          // 消息内容
                          R"( - .*:\d+\n)";  // 文件名:行号

    std::regex level_format_re(pattern);

    if (Logger::logLevel() <= stringToLogLevel(level)) {
      EXPECT_TRUE(std::regex_search(log_content, level_format_re))
          << level << " format should be correct\n"
          << "Actual log: " << log_content;
    } else {
      EXPECT_FALSE(std::regex_search(log_content, level_format_re))
          << level << " should not appear in log";
    }
  }
}
