add_subdirectory(./tool/)

# 设置输出目录
set(PROTO_OUT_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

add_library(rpc
  ./protobuf_codec_lite.cpp
  ./rpc_channel.cpp
  ./rpc_codec.cpp
)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/protos/*.proto")
# protobuf_generate(
#   TARGET rpc
#   PLUGIN "protoc-gen-cpp-plugin=$<TARGET_FILE:gen-cpp-plugin>"
#   IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/protos
#   PROTOC_OUT_DIR ${PROTO_OUT_DIR}
#   PROTOS ${PROTO_FILES}
# )

# protobuf_generate(
#   TARGET rpc
#   LANGUAGE cpp  # 生成标准 C++ 代码
#   PLUGIN gen-cpp-plugin
#   IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/protos
#   PROTOC_OUT_DIR ${PROTO_OUT_DIR}
#   PROTOS ${PROTO_FILES}
# )
protobuf_generate(
  TARGET rpc
  LANGUAGE cpp  # 生成标准 C++ 代码
  PLUGIN gen-cpp-plugin
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protos  # **更正: 使用 PROTO_PATH**
  PROTOC_OUTPUT_DIR ${PROTO_OUT_DIR} # **更正: 使用 PROTOC_OUTPUT_DIR**
  PROTOS ${PROTO_FILES}
)
# add_custom_command(
#   OUTPUT ${GENERATED_FILES}
#   COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
#   ARGS
#     --cpp_out=${PROTO_OUT_DIR}
#     --plugin=protoc-gen-cpp-plugin={CMAKE_BINARY_DIR}/bin/gen-cpp-plugin
#     --cpp-plugin_out=${PROTO_OUT_DIR}
#     -I${CMAKE_CURRENT_SOURCE_DIR}/protos
#     ${PROTO_FILES}
#   DEPENDS gen-cpp-plugin ${PROTO_FILES}
#   VERBATIM
# )

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
