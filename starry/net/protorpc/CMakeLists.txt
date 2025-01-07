add_library(protorpc)

target_include_directories(protorpc PUBLIC include)
target_sources(protorpc PRIVATE
  ./src/rpc_codec.cpp
)

target_link_libraries(protorpc PROVATE
  net 
  log 
  protobuf
  protobuf_codec
  z
)
