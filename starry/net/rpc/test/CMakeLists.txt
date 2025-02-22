add_executable(rpc_codec_test ./rpc_codec_test.cpp)
target_link_libraries(
  rpc_codec_test
  gtest
  GTest::gtest_main
  log
  copyable
  noncopyable
  net
  rpc
  absl::log_internal_check_op
  ${Protobuf_LIBRARIES}
)

add_executable(rpc_controller_test ./rpc_controller_test.cpp)
target_link_libraries(
  rpc_controller_test
  gtest
  GTest::gtest_main
  log
  copyable
  noncopyable
  net
  rpc
  absl::log_internal_check_op
  ${Protobuf_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(rpc_codec_test)
gtest_discover_tests(rpc_controller_test)
