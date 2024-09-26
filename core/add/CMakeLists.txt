# 添加静态库
add_library(add STATIC)

target_include_directories(add PUBLIC include/)

target_sources(add PRIVATE 
    src/add.cpp)

target_link_libraries(add PRIVATE
  noncopyable)

# 测试
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
