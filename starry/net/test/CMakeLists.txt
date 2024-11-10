add_executable(buffer_test buffer_test.cpp)
target_link_libraries(buffer_test 
  gtest 
  GTest::gtest_main
  log
  copyable
  noncopyable
  net)

add_executable(inet_address_test inet_address_test.cpp)
target_link_libraries(inet_address_test 
  gtest 
  GTest::gtest_main
  log
  copyable
  noncopyable
  net)

add_executable(socket_test socket_test.cpp)
target_link_libraries(socket_test 
  gtest 
  GTest::gtest_main
  log
  copyable
  noncopyable
  net)

include(GoogleTest)
gtest_discover_tests(buffer_test)
gtest_discover_tests(inet_address_test)
gtest_discover_tests(socket_test)
