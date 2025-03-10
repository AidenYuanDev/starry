#r设置输出目录
set(PROTO_OUT_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# 获取所有 proto 文件
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/protos/*.proto")

# 定义正确的插件路径 
set(PLUGIN_PATH ${CMAKE_BINARY_DIR}/bin/gen-cpp-plugin)

# 定义生成的文件
set(GENERATED_PB_FILES
    ${PROTO_OUT_DIR}/resolver.pb.cc 
    ${PROTO_OUT_DIR}/resolver.pb.h 
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

# 定义 resolver 库，并包含生成的文件
add_executable(resolver
  ./client.cpp
  ${GENERATED_PB_FILES}  # 将生成的文件添加到库中
)

# 设置包含目录
target_include_directories(resolver PUBLIC
  .
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Protobuf_INCLUDE_DIRS}
  ${PROTO_OUT_DIR}
)

# 链接库
target_link_libraries(resolver PUBLIC
  ${Protobuf_LIBRARIES}
  PkgConfig::ABSL
  # ZLIB::ZLIB
  ${ZLIB_LIBRARIES}
  log
  net 
  net 
  rpc
)
