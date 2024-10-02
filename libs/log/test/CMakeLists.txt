add_executable(logger_test ./logger_test.cpp)

target_link_libraries(logger_test
    PRIVATE
        GTest::gtest_main
        log
        copyable
        noncopyable
)

add_executable(log_performance_test ./log_performance_test.cpp)

target_link_libraries(log_performance_test
    PRIVATE
        GTest::gtest_main
        log
        copyable
        noncopyable
)

include(GoogleTest)
gtest_discover_tests(logger_test)
gtest_discover_tests(log_performance_test)

