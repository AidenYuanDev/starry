# 添加静态库
add_library(net)

target_include_directories(net PUBLIC include)

target_sources(net PRIVATE
  ./src/epoll_poller.cpp
  ./src/channel.cpp
  ./src/sockets_ops.cpp
  ./src/eventloop.cpp
  ./src/eventloop_thread.cpp
  ./src/eventloop_threadpool.cpp
  ./src/inet_address.cpp
  ./src/buffer.cpp
  ./src/tcp_connection.cpp
  ./src/socket.cpp
  ./src/tcp_server.cpp
  ./src/acceptor.cpp
  ./src/tcp_client.cpp
  ./src/timer.cpp
  ./src/timer_queue.cpp
)

target_link_libraries(net PRIVATE
  log
  copyable 
  noncopyable)

# 测试
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
