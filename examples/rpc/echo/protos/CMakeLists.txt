# Proto定义与生成
set(PROTO_FILES
    echo.proto
)

# 输出目录
set(PROTO_GEN_DIR ${CMAKE_BINARY_DIR}/generated/echo)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# 生成protobuf文件
add_custom_command(
    OUTPUT 
        ${PROTO_GEN_DIR}/echo.pb.cc 
        ${PROTO_GEN_DIR}/echo.pb.h
    COMMAND protoc 
        --cpp_out=${PROTO_GEN_DIR} 
        --plugin=protoc-gen-cpp-plugin=${CMAKE_BINARY_DIR}/bin/gen-cpp-plugin
        --cpp-plugin_out=${PROTO_GEN_DIR} 
        -I${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/echo.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/echo.proto
    COMMENT "Generating echo protocol buffer files"
    VERBATIM
)

# 创建库
add_library(echo_proto STATIC
    ${PROTO_GEN_DIR}/echo.pb.cc
)

target_include_directories(echo_proto PUBLIC
    ${PROTO_GEN_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(echo_proto PUBLIC
    ${Protobuf_LIBRARIES}
    rpc
)
