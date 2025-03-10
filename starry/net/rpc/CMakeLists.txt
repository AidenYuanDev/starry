# 生成插件
add_subdirectory(./tool/)
# 设置输出目录
set(PROTO_OUT_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# 获取所有 proto 文件
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/protos/*.proto")

# 定义正确的插件路径 
set(PLUGIN_PATH ${CMAKE_BINARY_DIR}/bin/gen-cpp-plugin)

# 定义生成的文件
set(GENERATED_PB_FILES
    ${PROTO_OUT_DIR}/a.pb.cc 
    ${PROTO_OUT_DIR}/a.pb.h 
    ${PROTO_OUT_DIR}/rpc.pb.cc 
    ${PROTO_OUT_DIR}/rpc.pb.h 
    ${PROTO_OUT_DIR}/rpcservice.pb.cc 
    ${PROTO_OUT_DIR}/rpcservice.pb.h 
)

# 自定义命令生成 protobuf 文件
add_custom_command(
    OUTPUT ${GENERATED_PB_FILES}
    COMMAND protoc 
            --cpp_out=${PROTO_OUT_DIR} 
            --plugin=protoc-gen-cpp-plugin=${PLUGIN_PATH}
            --cpp-plugin_out=${PROTO_OUT_DIR} 
            -I${CMAKE_CURRENT_SOURCE_DIR}/protos/
            -I${PROTO_OUT_DIR}
            ${PROTO_FILES}
    DEPENDS ${PROTO_FILES} ${PLUGIN_PATH}
    COMMENT "Generating protocol buffer files"
    VERBATIM
)

# 定义 rpc 库，并包含生成的文件
add_library(rpc
  ./protobuf_codec_lite.cpp
  ./rpc_codec.cpp
  ./rpc_channel.cpp
  ./rpc_service.cpp
  ./rpc_server.cpp
  ${GENERATED_PB_FILES}  # 将生成的文件添加到库中
)

# 设置包含目录
target_include_directories(rpc PUBLIC
  .
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Protobuf_INCLUDE_DIRS}
  ${PROTO_OUT_DIR}
)

# 链接库
target_link_libraries(rpc PUBLIC
  ${Protobuf_LIBRARIES}
  ZLIB::ZLIB
  PkgConfig::ABSL
  net 
  log
)
