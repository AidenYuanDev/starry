add_subdirectory(./tool/)

# 设置输出目录
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

add_library(rpc
  ./protobuf_codec_lite.cpp
  ./rpc_channel.cpp
  ./rpc_codec.cpp
)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/protos/*.proto")
protobuf_generate(
  TARGET rpc
  PLUGIN "protoc-gen-cpp-plugin=$<TARGET_FILE:gen-cpp-plugin>"
  IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/protos
  PROTOC_OUT_DIR ${PROTO_OUT_DIR}
  PROTOS ${PROTO_FILES}
)

target_include_directories(rpc PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Protobuf_INCLUDE_DIRS}
  ${PROTO_OUT_DIR}
)

target_link_libraries(rpc PUBLIC
  ${Protobuf_LIBRARIES}
  PkgConfig::ABSL
  net 
  log
)
