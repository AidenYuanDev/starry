# 添加静态库
add_library(thread_pool INTERFACE)

target_include_directories(thread_pool INTERFACE include)

# 测试
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
