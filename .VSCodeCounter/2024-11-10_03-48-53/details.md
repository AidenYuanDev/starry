# Details

Date : 2024-11-10 03:48:53

Directory /home/aiden/Development/starry/starry

Total : 62 files,  3819 codes, 279 comments, 804 blanks, all 4902 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [starry/CMakeLists.txt](/starry/CMakeLists.txt) | CMake | 10 | 0 | 3 | 13 |
| [starry/base/CMakeLists.txt](/starry/base/CMakeLists.txt) | CMake | 12 | 0 | 2 | 14 |
| [starry/base/copyable/CMakeLists.txt](/starry/base/copyable/CMakeLists.txt) | CMake | 3 | 0 | 2 | 5 |
| [starry/base/copyable/include/copyable.h](/starry/base/copyable/include/copyable.h) | C++ | 3 | 0 | 3 | 6 |
| [starry/base/log/CMakeLists.txt](/starry/base/log/CMakeLists.txt) | CMake | 15 | 0 | 5 | 20 |
| [starry/base/log/include/async_logging.h](/starry/base/log/include/async_logging.h) | C++ | 39 | 1 | 10 | 50 |
| [starry/base/log/include/log_file.h](/starry/base/log/include/log_file.h) | C++ | 36 | 1 | 10 | 47 |
| [starry/base/log/include/log_stream.h](/starry/base/log/include/log_stream.h) | C++ | 62 | 5 | 14 | 81 |
| [starry/base/log/include/logging.h](/starry/base/log/include/logging.h) | C++ | 42 | 4 | 16 | 62 |
| [starry/base/log/src/async_logger.cpp](/starry/base/log/src/async_logger.cpp) | C++ | 96 | 4 | 18 | 118 |
| [starry/base/log/src/log_file.cpp](/starry/base/log/src/log_file.cpp) | C++ | 85 | 6 | 10 | 101 |
| [starry/base/log/src/logging.cpp](/starry/base/log/src/logging.cpp) | C++ | 71 | 4 | 17 | 92 |
| [starry/base/log/test/CMakeLists.txt](/starry/base/log/test/CMakeLists.txt) | CMake | 19 | 0 | 6 | 25 |
| [starry/base/log/test/log_performance_test.cpp](/starry/base/log/test/log_performance_test.cpp) | C++ | 115 | 11 | 34 | 160 |
| [starry/base/log/test/logger_test.cpp](/starry/base/log/test/logger_test.cpp) | C++ | 70 | 9 | 21 | 100 |
| [starry/base/noncopyable/CMakeLists.txt](/starry/base/noncopyable/CMakeLists.txt) | CMake | 4 | 0 | 2 | 6 |
| [starry/base/noncopyable/include/noncopyable.h](/starry/base/noncopyable/include/noncopyable.h) | C++ | 11 | 0 | 3 | 14 |
| [starry/base/thread_pool/CMakeLists.txt](/starry/base/thread_pool/CMakeLists.txt) | CMake | 7 | 0 | 3 | 10 |
| [starry/base/thread_pool/include/thread_pool.h](/starry/base/thread_pool/include/thread_pool.h) | C++ | 106 | 0 | 19 | 125 |
| [starry/base/thread_pool/test/CMakeLists.txt](/starry/base/thread_pool/test/CMakeLists.txt) | CMake | 8 | 0 | 4 | 12 |
| [starry/base/thread_pool/test/thread_pool_test.cpp](/starry/base/thread_pool/test/thread_pool_test.cpp) | C++ | 48 | 1 | 16 | 65 |
| [starry/net/CMakeLists.txt](/starry/net/CMakeLists.txt) | CMake | 29 | 0 | 5 | 34 |
| [starry/net/include/acceptor.h](/starry/net/include/acceptor.h) | C++ | 30 | 1 | 10 | 41 |
| [starry/net/include/buffer.h](/starry/net/include/buffer.h) | C++ | 210 | 31 | 30 | 271 |
| [starry/net/include/callbacks.h](/starry/net/include/callbacks.h) | C++ | 23 | 0 | 5 | 28 |
| [starry/net/include/channel.h](/starry/net/include/channel.h) | C++ | 58 | 6 | 17 | 81 |
| [starry/net/include/connector.h](/starry/net/include/connector.h) | C++ | 49 | 1 | 14 | 64 |
| [starry/net/include/epoll_poller.h](/starry/net/include/epoll_poller.h) | C++ | 25 | 3 | 11 | 39 |
| [starry/net/include/eventloop.h](/starry/net/include/eventloop.h) | C++ | 68 | 13 | 22 | 103 |
| [starry/net/include/eventloop_thread.h](/starry/net/include/eventloop_thread.h) | C++ | 27 | 1 | 10 | 38 |
| [starry/net/include/eventloop_threadpool.h](/starry/net/include/eventloop_threadpool.h) | C++ | 30 | 2 | 11 | 43 |
| [starry/net/include/inet_address.h](/starry/net/include/inet_address.h) | C++ | 35 | 8 | 11 | 54 |
| [starry/net/include/poller.h](/starry/net/include/poller.h) | C++ | 28 | 2 | 11 | 41 |
| [starry/net/include/socket.h](/starry/net/include/socket.h) | C++ | 25 | 0 | 10 | 35 |
| [starry/net/include/sockets_ops.h](/starry/net/include/sockets_ops.h) | C++ | 33 | 3 | 10 | 46 |
| [starry/net/include/tcp_client.h](/starry/net/include/tcp_client.h) | C++ | 48 | 0 | 15 | 63 |
| [starry/net/include/tcp_connection.h](/starry/net/include/tcp_connection.h) | C++ | 93 | 9 | 17 | 119 |
| [starry/net/include/tcp_server.h](/starry/net/include/tcp_server.h) | C++ | 61 | 0 | 15 | 76 |
| [starry/net/include/timer.h](/starry/net/include/timer.h) | C++ | 31 | 0 | 9 | 40 |
| [starry/net/include/timer_id.h](/starry/net/include/timer_id.h) | C++ | 14 | 0 | 7 | 21 |
| [starry/net/include/timer_queue.h](/starry/net/include/timer_queue.h) | C++ | 37 | 0 | 12 | 49 |
| [starry/net/src/acceptor.cpp](/starry/net/src/acceptor.cpp) | C++ | 56 | 2 | 7 | 65 |
| [starry/net/src/buffer.cpp](/starry/net/src/buffer.cpp) | C++ | 30 | 1 | 7 | 38 |
| [starry/net/src/channel.cpp](/starry/net/src/channel.cpp) | C++ | 93 | 0 | 18 | 111 |
| [starry/net/src/connector.cpp](/starry/net/src/connector.cpp) | C++ | 165 | 13 | 23 | 201 |
| [starry/net/src/epoll_poller.cpp](/starry/net/src/epoll_poller.cpp) | C++ | 140 | 6 | 16 | 162 |
| [starry/net/src/eventloop.cpp](/starry/net/src/eventloop.cpp) | C++ | 186 | 16 | 29 | 231 |
| [starry/net/src/eventloop_thread.cpp](/starry/net/src/eventloop_thread.cpp) | C++ | 39 | 2 | 13 | 54 |
| [starry/net/src/eventloop_threadpool.cpp](/starry/net/src/eventloop_threadpool.cpp) | C++ | 56 | 4 | 13 | 73 |
| [starry/net/src/inet_address.cpp](/starry/net/src/inet_address.cpp) | C++ | 77 | 8 | 13 | 98 |
| [starry/net/src/poller.cpp](/starry/net/src/poller.cpp) | C++ | 15 | 0 | 5 | 20 |
| [starry/net/src/socket.cpp](/starry/net/src/socket.cpp) | C++ | 80 | 0 | 14 | 94 |
| [starry/net/src/sockets_ops.cpp](/starry/net/src/sockets_ops.cpp) | C++ | 200 | 19 | 26 | 245 |
| [starry/net/src/tcp_client.cpp](/starry/net/src/tcp_client.cpp) | C++ | 107 | 1 | 15 | 123 |
| [starry/net/src/tcp_connection.cpp](/starry/net/src/tcp_connection.cpp) | C++ | 271 | 26 | 32 | 329 |
| [starry/net/src/tcp_server.cpp](/starry/net/src/tcp_server.cpp) | C++ | 85 | 5 | 12 | 102 |
| [starry/net/src/timer.cpp](/starry/net/src/timer.cpp) | C++ | 14 | 0 | 4 | 18 |
| [starry/net/src/timer_queue.cpp](/starry/net/src/timer_queue.cpp) | C++ | 173 | 0 | 30 | 203 |
| [starry/net/test/CMakeLists.txt](/starry/net/test/CMakeLists.txt) | CMake | 28 | 0 | 4 | 32 |
| [starry/net/test/buffer_test.cpp](/starry/net/test/buffer_test.cpp) | C++ | 63 | 18 | 20 | 101 |
| [starry/net/test/inet_address_test.cpp](/starry/net/test/inet_address_test.cpp) | C++ | 60 | 6 | 15 | 81 |
| [starry/net/test/socket_test.cpp](/starry/net/test/socket_test.cpp) | C++ | 65 | 26 | 18 | 109 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)