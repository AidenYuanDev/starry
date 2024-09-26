add_library(sub STATIC)

target_include_directories(sub PUBLIC include/)

target_sources(sub PRIVATE 
    src/sub.cpp)

target_link_libraries(sub PRIVATE
  noncopyable)
