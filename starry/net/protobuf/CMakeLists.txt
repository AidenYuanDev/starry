add_library(protobuf_codec)

target_include_directories(protobuf_codec PUBLIC include)
target_sources(protobuf_codec PRIVATE
  ./src/protobuf_codeclite.cpp 
)

target_link_libraries(protobuf_codec PROVATE
  net 
  log 
  protobuf
  z
)
