# Echo服务器
add_executable(echo_server
    echo_server.cpp
)

target_include_directories(echo_server PRIVATE
    ${PROTO_GEN_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(echo_server
    ZLIB::ZLIB
    echo_proto
    rpc
    net
    log
    noncopyable
    ${Protobuf_LIBRARIES}
)
