set(PROTO_FILES
  ./proto/rpc_message.proto)

# 生成 RPC 框架代码

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(rpc)

target_include_directories(rpc PUBLIC
  .
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(rpc PRIVATE
  ${PROTO_HDRS}
  ${PROTO_SRCS}
  ./codec.cpp
)

target_link_libraries(rpc PRIVATE
  net
  log
  ${Protobuf_LIBRARIES}
)
