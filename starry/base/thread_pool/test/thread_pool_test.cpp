#include <gtest/gtest.h>
#include <atomic>
#include "thread_pool.h"

using namespace starry;

class ThreadPoolTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ThreadPoolTest, BasicFunctionality) {
  ThreadPool pool(4);
  std::vector<std::future<int>> results;

  for (int i = 0; i < 8; ++i) {
    results.emplace_back(
        pool.enqueue([](int answer) -> int { return answer; }, i));
  }

  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(results[i].get(), i);
  }
}

TEST_F(ThreadPoolTest, ConcurrencyTest) {
  ThreadPool pool(4);
  std::atomic<int> counter(0);
  std::vector<std::future<void>> futures;

  for (int i = 0; i < 1000; ++i) {
    futures.emplace_back(pool.enqueue([&counter] { ++counter; }));
  }

  for (auto& future : futures) {
    future.wait();
  }

  EXPECT_EQ(counter.load(), 1000);
}

TEST_F(ThreadPoolTest, StopAndRestart) {
  ThreadPool pool(4);
  std::atomic<int> counter(0);

  for (int i = 0; i < 100; ++i) {
    pool.enqueue([&counter] { ++counter; });
  }

  pool.stop();
  EXPECT_EQ(counter.load(), 100);

  // 重新启动
  pool.start();
  counter.store(0);

  for (int i = 0; i < 100; ++i) {
    pool.enqueue([&counter] { ++counter; });
  }

  pool.stop();
  EXPECT_EQ(counter.load(), 100);
}
