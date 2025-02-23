# 添加生成规则
set(PROTOC_GEN_RPC ${CMAKE_BINARY_DIR}/tools/protoc-gen-rpc)

# Echo 服务示例
set(ECHO_PROTO_FILES
  ./proto/echo_service.proto
)

protobuf_generate_cpp(ECHO_PROTO_SRCS ECHO_PROTO_HDRS ${ECHO_PROTO_FILES})

# 添加自定义命令生成 RPC 代码
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/echo_service.EchoService.rpc.h
    COMMAND protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
            --plugin=protoc-gen-rpc=${PROTOC_GEN_RPC}
            --rpc_out=${CMAKE_CURRENT_BINARY_DIR}
            ${ECHO_PROTO_FILES}
    DEPENDS ${ECHO_PROTO_FILES} protoc-gen-rpc
    COMMENT "Generating RPC stubs for ${ECHO_PROTO_FILES}"
)
add_custom_target(echo_rpc_stubs DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/echo_service.EchoService.rpc.h)


# 包含生成的头文件目录
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Echo 服务器
add_executable(echo_server
  ./echo_server.cpp
  ${ECHO_PROTO_SRCS})

add_dependencies(echo_server echo_rpc_stubs)

target_link_libraries(echo_server
  rpc
  net
  absl::log_internal_check_op
  log
  ${PROTOBUF_LIBRARIES})

# Echo 客户端
add_executable(echo_client
  ./echo_client.cpp
  ${ECHO_PROTO_SRCS})

add_dependencies(echo_client echo_rpc_stubs)

target_link_libraries(echo_client
  rpc
  net
  absl::log_internal_check_op
  log
  ${PROTOBUF_LIBRARIES}
)
