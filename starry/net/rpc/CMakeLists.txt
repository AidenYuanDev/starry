set(PROTO_FILES
  ./rpc.proto
)

# 生成 RPC 框架代码

protobuf_generate_cpp(RPC_SRCS RPC_HDRS ${PROTO_FILES})

add_library(rpc)

target_include_directories(rpc PUBLIC
  .
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(rpc PRIVATE
  ${RPC_HDRS}
  ${RPC_SRCS}
  ./protobuf_codec_lite.cpp
)

target_link_libraries(rpc PRIVATE
  net
  log
  ${Protobuf_LIBRARIES}
)

# add_subdirectory(./tools/)

# 测试
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
