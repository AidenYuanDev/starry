# 添加静态库
add_library(log)

target_include_directories(log PUBLIC include)

target_sources(log PRIVATE
  ./src/logging.cpp
  ./src/log_file.cpp
  ./src/async_logger.cpp
)

target_link_libraries(log PRIVATE
  copyable 
  noncopyable)

# 测试
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
