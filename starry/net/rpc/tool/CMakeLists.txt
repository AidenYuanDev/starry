# 1. 构建 Protobuf 插件
add_executable(gen-cpp-plugin
  ./cpp_plugin.cpp
  ./cpp_service.cpp
)

target_include_directories(gen-cpp-plugin PRIVATE ..
  ${Protobuf_INCLUDE_DIRS}
  ${ABSL_INCLUDE_DIRS}
)

target_link_libraries(gen-cpp-plugin PRIVATE
  ${Protobuf_LIBRARIES}
  ${Protobuf_PROTOC_LIBRARIES}  # 添加 Protobuf 编译器库
  PkgConfig::ABSL
  net 
  log
)

# # 2. 设置 Proto 生成目录
# set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen-cpp-plugin)
# file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# # 3. 定义 Proto 文件
# set(PROTO_FILES
#   ${CMAKE_CURRENT_SOURCE_DIR}/rpc.proto
#   ${CMAKE_CURRENT_SOURCE_DIR}/rpcservice.proto
# )

# 4. 创建库 target
# add_library(gen-cpp-plugin-lib)
#
# target_include_directories(gen-cpp-plugin-lib PUBLIC
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
#   $<BUILD_INTERFACE:${PROTO_GEN_DIR}>
#   $<INSTALL_INTERFACE:include>
#   ${Protobuf_INCLUDE_DIRS}
# )
#
# target_link_libraries(gen-cpp-plugin-lib PUBLIC
#   ${Protobuf_LIBRARIES}
#   PkgConfig::ABSL
# )

# 创建辅助函数
# function(generate_rpc_code TARGET_NAME)
#   # 解析可选参数
#   set(options "")
#   set(oneValueArgs OUTPUT_DIR)
#   set(multiValueArgs PROTOS IMPORT_DIRS)
#   cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
#   
#   # 设置默认值
#   if(NOT ARG_OUTPUT_DIR)
#     set(ARG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
#   endif()
#   if(NOT ARG_IMPORT_DIRS)
#     set(ARG_IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
#   endif()
#   
#   # 确保输出目录存在
#   file(MAKE_DIRECTORY ${ARG_OUTPUT_DIR})
#
#   # 使用 protobuf_generate
#   protobuf_generate(
#     TARGET ${TARGET_NAME}
#     PLUGIN "protoc-gen-cpp-plugin=${PATH_TO_PLUGIN}/gen-cpp-plugin"
#     GENERATE_EXTENSIONS .pb.h .pb.cc 
#     IMPORT_DIRS ${ARG_IMPORT_DIRS}
#     PROTOC_OUT_DIR ${ARG_OUTPUT_DIR}
#     PROTOS ${ARG_PROTOS}
#   )
#   
#   # 添加包含目录
#   target_include_directories(${TARGET_NAME} PUBLIC ${ARG_OUTPUT_DIR})
# endfunction()
