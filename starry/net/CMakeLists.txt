# 添加静态库
add_library(net)

target_include_directories(net PUBLIC include)

target_sources(net PRIVATE
  ./src/poller.cpp
  ./src/epoll_poller.cpp
  ./src/channel.cpp
  ./src/sockets_ops.cpp
  ./src/eventlopp.cpp
)

target_link_libraries(net PRIVATE
  log
  copyable 
  noncopyable)

# 测试
# if(BUILD_TESTING)
#   add_subdirectory(test)
# endif()
