# 添加静态库
add_library(noncopyable INTERFACE)

target_include_directories(noncopyable INTERFACE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(noncopyable INTERFACE
  include/noncopyable.h)

target_link_libraries(noncopyable PRIVATE)

# 测试
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
