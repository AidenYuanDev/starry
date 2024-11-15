#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

// 包含您的日志系统头文件
#include "async_logging.h"
#include "logging.h"

// 使用您的命名空间
using namespace starry;

// 辅助函数：生成指定大小的随机字符串
std::string generateRandomString(size_t length) {
  const char charset[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string result;
  result.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    result += charset[rand() % (sizeof(charset) - 1)];
  }
  return result;
}

class LogPerformanceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // 初始化异步日志系统
    asyncLog_ =
        std::make_unique<AsyncLogging>("test_log", "/tmp", 64 * 1024 * 1024);
    asyncLog_->start();

    // 设置日志输出到异步日志
    Logger::setOutput(
        [this](const char* msg, int len) { asyncLog_->append(msg, len); });

    // 设置日志级别
    Logger::setLogLevel(LogLevel::INFO);
  }

  void TearDown() override {
    // 停止异步日志
    asyncLog_->stop();
  }

  std::unique_ptr<AsyncLogging> asyncLog_;
};

// 测试 1: 吞吐量测试
TEST_F(LogPerformanceTest, ThroughputTest) {
  const int NUM_LOGS = 1000000;
  const std::string message =
      "This is a test log message for throughput measurement";

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_LOGS; ++i) {
    LOG_INFO << message;
  }
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  double throughput = static_cast<double>(NUM_LOGS) / (duration / 1000.0);

  std::cout << "Throughput Test:\n"
            << "Total logs: " << NUM_LOGS << "\n"
            << "Total time: " << duration << " ms\n"
            << "Throughput: " << throughput << " logs/second\n";

  EXPECT_GE(throughput, 100000);
}

// 测试 2: 延迟测试
TEST_F(LogPerformanceTest, LatencyTest) {
  const int NUM_LOGS = 10000;
  const std::string message =
      "This is a test log message for latency measurement";
  std::vector<long long> latencies;

  for (int i = 0; i < NUM_LOGS; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    LOG_INFO << message;
    auto end = std::chrono::high_resolution_clock::now();
    latencies.push_back(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count());
  }

  std::sort(latencies.begin(), latencies.end());
  long long p50 = latencies[NUM_LOGS / 2];
  long long p99 = latencies[NUM_LOGS * 99 / 100];

  std::cout << "Latency Test:\n"
            << "50th percentile latency: " << p50 << " microseconds\n"
            << "99th percentile latency: " << p99 << " microseconds\n";

  EXPECT_LE(p50, 10);
  EXPECT_LE(p99, 100);
}

// 测试 3: 多线程性能测试
TEST_F(LogPerformanceTest, MultiThreadedPerformance) {
  const int NUM_THREADS = 8;
  const int LOGS_PER_THREAD = 100000;
  const std::string message = "This is a multi-threaded log message";

  std::atomic<int> total_logs(0);
  auto start = std::chrono::high_resolution_clock::now();

  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back([&]() {
      for (int j = 0; j < LOGS_PER_THREAD; ++j) {
        LOG_INFO << message;
        total_logs.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();

  double throughput = static_cast<double>(total_logs) / (duration / 1000.0);

  std::cout << "Multi-threaded Performance Test:\n"
            << "Total logs: " << total_logs << "\n"
            << "Total time: " << duration << " ms\n"
            << "Throughput: " << throughput << " logs/second\n";

  EXPECT_GE(throughput, 400000);
}

// 测试 4: 持续写入测试
TEST_F(LogPerformanceTest, SustainedWriteTest) {
  const int DURATION_SECONDS = 60;
  const std::string message = "This is a sustained write test message";
  std::atomic<long long> log_count(0);

  auto end_time =
      std::chrono::steady_clock::now() + std::chrono::seconds(DURATION_SECONDS);

  while (std::chrono::steady_clock::now() < end_time) {
    LOG_INFO << message;
    log_count.fetch_add(1, std::memory_order_relaxed);
  }

  double avg_throughput = static_cast<double>(log_count) / DURATION_SECONDS;

  std::cout << "Sustained Write Test:\n"
            << "Duration: " << DURATION_SECONDS << " seconds\n"
            << "Total logs: " << log_count << "\n"
            << "Average throughput: " << avg_throughput << " logs/second\n";

  EXPECT_GE(avg_throughput, 50000);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
