# Echo客户端
add_executable(echo_client
    echo_client.cpp
)

target_include_directories(echo_client PRIVATE
    ${PROTO_GEN_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(echo_client
    ZLIB::ZLIB
    echo_proto
    rpc
    net
    log
    noncopyable
    ${Protobuf_LIBRARIES}
)
