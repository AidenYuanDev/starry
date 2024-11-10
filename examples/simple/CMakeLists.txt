add_executable(simple_echo
  ./src/echo.cpp
  ./src/main.cpp)
target_include_directories(simple_echo PRIVATE ./include/)
target_link_libraries(simple_echo
  net
  log 
  noncopyable
)
