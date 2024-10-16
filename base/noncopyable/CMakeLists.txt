# 添加静态库
add_library(noncopyable INTERFACE)

target_include_directories(noncopyable
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
