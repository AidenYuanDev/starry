# Details

Date : 2024-11-22 03:28:40

Directory /home/aiden/Development/starry/starry

Total : 60 files,  3916 codes, 313 comments, 817 blanks, all 5046 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [starry/CMakeLists.txt](/starry/CMakeLists.txt) | CMake | 11 | 0 | 4 | 15 |
| [starry/base/CMakeLists.txt](/starry/base/CMakeLists.txt) | CMake | 12 | 0 | 2 | 14 |
| [starry/base/copyable/CMakeLists.txt](/starry/base/copyable/CMakeLists.txt) | CMake | 3 | 0 | 2 | 5 |
| [starry/base/copyable/include/copyable.h](/starry/base/copyable/include/copyable.h) | C++ | 3 | 0 | 3 | 6 |
| [starry/base/log/CMakeLists.txt](/starry/base/log/CMakeLists.txt) | CMake | 13 | 0 | 5 | 18 |
| [starry/base/log/include/async_logging.h](/starry/base/log/include/async_logging.h) | C++ | 36 | 1 | 10 | 47 |
| [starry/base/log/include/log_file.h](/starry/base/log/include/log_file.h) | C++ | 34 | 1 | 10 | 45 |
| [starry/base/log/include/log_stream.h](/starry/base/log/include/log_stream.h) | C++ | 61 | 5 | 13 | 79 |
| [starry/base/log/include/logging.h](/starry/base/log/include/logging.h) | C++ | 47 | 4 | 16 | 67 |
| [starry/base/log/src/async_logger.cpp](/starry/base/log/src/async_logger.cpp) | C++ | 99 | 4 | 18 | 121 |
| [starry/base/log/src/log_file.cpp](/starry/base/log/src/log_file.cpp) | C++ | 83 | 6 | 10 | 99 |
| [starry/base/log/src/logging.cpp](/starry/base/log/src/logging.cpp) | C++ | 79 | 4 | 14 | 97 |
| [starry/base/log/test/CMakeLists.txt](/starry/base/log/test/CMakeLists.txt) | CMake | 19 | 0 | 6 | 25 |
| [starry/base/log/test/log_performance_test.cpp](/starry/base/log/test/log_performance_test.cpp) | C++ | 125 | 11 | 34 | 170 |
| [starry/base/log/test/logger_test.cpp](/starry/base/log/test/logger_test.cpp) | C++ | 171 | 23 | 49 | 243 |
| [starry/base/noncopyable/CMakeLists.txt](/starry/base/noncopyable/CMakeLists.txt) | CMake | 4 | 0 | 2 | 6 |
| [starry/base/noncopyable/include/noncopyable.h](/starry/base/noncopyable/include/noncopyable.h) | C++ | 11 | 0 | 3 | 14 |
| [starry/base/thread_pool/CMakeLists.txt](/starry/base/thread_pool/CMakeLists.txt) | CMake | 7 | 0 | 3 | 10 |
| [starry/base/thread_pool/include/thread_pool.h](/starry/base/thread_pool/include/thread_pool.h) | C++ | 106 | 0 | 19 | 125 |
| [starry/base/thread_pool/test/CMakeLists.txt](/starry/base/thread_pool/test/CMakeLists.txt) | CMake | 8 | 0 | 4 | 12 |
| [starry/base/thread_pool/test/thread_pool_test.cpp](/starry/base/thread_pool/test/thread_pool_test.cpp) | C++ | 48 | 1 | 16 | 65 |
| [starry/net/CMakeLists.txt](/starry/net/CMakeLists.txt) | CMake | 28 | 0 | 5 | 33 |
| [starry/net/include/acceptor.h](/starry/net/include/acceptor.h) | C++ | 30 | 1 | 10 | 41 |
| [starry/net/include/buffer.h](/starry/net/include/buffer.h) | C++ | 210 | 31 | 30 | 271 |
| [starry/net/include/callbacks.h](/starry/net/include/callbacks.h) | C++ | 24 | 0 | 5 | 29 |
| [starry/net/include/channel.h](/starry/net/include/channel.h) | C++ | 58 | 6 | 17 | 81 |
| [starry/net/include/connector.h](/starry/net/include/connector.h) | C++ | 49 | 1 | 14 | 64 |
| [starry/net/include/epoll_poller.h](/starry/net/include/epoll_poller.h) | C++ | 32 | 3 | 12 | 47 |
| [starry/net/include/eventloop.h](/starry/net/include/eventloop.h) | C++ | 68 | 13 | 22 | 103 |
| [starry/net/include/eventloop_thread.h](/starry/net/include/eventloop_thread.h) | C++ | 27 | 1 | 10 | 38 |
| [starry/net/include/eventloop_threadpool.h](/starry/net/include/eventloop_threadpool.h) | C++ | 30 | 2 | 11 | 43 |
| [starry/net/include/inet_address.h](/starry/net/include/inet_address.h) | C++ | 35 | 8 | 11 | 54 |
| [starry/net/include/socket.h](/starry/net/include/socket.h) | C++ | 25 | 0 | 10 | 35 |
| [starry/net/include/sockets_ops.h](/starry/net/include/sockets_ops.h) | C++ | 33 | 3 | 10 | 46 |
| [starry/net/include/tcp_client.h](/starry/net/include/tcp_client.h) | C++ | 48 | 0 | 15 | 63 |
| [starry/net/include/tcp_connection.h](/starry/net/include/tcp_connection.h) | C++ | 93 | 9 | 17 | 119 |
| [starry/net/include/tcp_server.h](/starry/net/include/tcp_server.h) | C++ | 61 | 0 | 15 | 76 |
| [starry/net/include/timer.h](/starry/net/include/timer.h) | C++ | 32 | 0 | 10 | 42 |
| [starry/net/include/timer_id.h](/starry/net/include/timer_id.h) | C++ | 14 | 0 | 7 | 21 |
| [starry/net/include/timer_queue.h](/starry/net/include/timer_queue.h) | C++ | 38 | 4 | 12 | 54 |
| [starry/net/src/acceptor.cpp](/starry/net/src/acceptor.cpp) | C++ | 56 | 2 | 7 | 65 |
| [starry/net/src/buffer.cpp](/starry/net/src/buffer.cpp) | C++ | 30 | 1 | 7 | 38 |
| [starry/net/src/channel.cpp](/starry/net/src/channel.cpp) | C++ | 93 | 0 | 18 | 111 |
| [starry/net/src/connector.cpp](/starry/net/src/connector.cpp) | C++ | 167 | 13 | 23 | 203 |
| [starry/net/src/epoll_poller.cpp](/starry/net/src/epoll_poller.cpp) | C++ | 136 | 6 | 16 | 158 |
| [starry/net/src/eventloop.cpp](/starry/net/src/eventloop.cpp) | C++ | 184 | 16 | 29 | 229 |
| [starry/net/src/eventloop_thread.cpp](/starry/net/src/eventloop_thread.cpp) | C++ | 39 | 2 | 13 | 54 |
| [starry/net/src/eventloop_threadpool.cpp](/starry/net/src/eventloop_threadpool.cpp) | C++ | 56 | 4 | 13 | 73 |
| [starry/net/src/inet_address.cpp](/starry/net/src/inet_address.cpp) | C++ | 77 | 8 | 13 | 98 |
| [starry/net/src/socket.cpp](/starry/net/src/socket.cpp) | C++ | 80 | 0 | 14 | 94 |
| [starry/net/src/sockets_ops.cpp](/starry/net/src/sockets_ops.cpp) | C++ | 200 | 19 | 26 | 245 |
| [starry/net/src/tcp_client.cpp](/starry/net/src/tcp_client.cpp) | C++ | 108 | 1 | 15 | 124 |
| [starry/net/src/tcp_connection.cpp](/starry/net/src/tcp_connection.cpp) | C++ | 271 | 27 | 32 | 330 |
| [starry/net/src/tcp_server.cpp](/starry/net/src/tcp_server.cpp) | C++ | 85 | 5 | 12 | 102 |
| [starry/net/src/timer.cpp](/starry/net/src/timer.cpp) | C++ | 14 | 0 | 4 | 18 |
| [starry/net/src/timer_queue.cpp](/starry/net/src/timer_queue.cpp) | C++ | 186 | 17 | 32 | 235 |
| [starry/net/test/CMakeLists.txt](/starry/net/test/CMakeLists.txt) | CMake | 31 | 0 | 4 | 35 |
| [starry/net/test/buffer_test.cpp](/starry/net/test/buffer_test.cpp) | C++ | 63 | 18 | 20 | 101 |
| [starry/net/test/inet_address_test.cpp](/starry/net/test/inet_address_test.cpp) | C++ | 60 | 6 | 15 | 81 |
| [starry/net/test/socket_test.cpp](/starry/net/test/socket_test.cpp) | C++ | 65 | 26 | 18 | 109 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)