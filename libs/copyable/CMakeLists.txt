# 添加静态库
add_library(copyable INTERFACE)

target_include_directories(copyable
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
