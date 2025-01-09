add_library(protorpc
        src/rpc_channel.cpp)

target_include_directories(protorpc PUBLIC include)
target_sources(protorpc PRIVATE
)

target_link_libraries(protorpc PROVATE
  net 
  log 
  protobuf
  protobuf_codec
  z
)
