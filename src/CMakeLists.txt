add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    add 
    sub
    noncopyable)

# 根据构建类型设置选项
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:-g -O0>
  $<$<CONFIG:Release>:-O3>
  $<$<CONFIG:RelWithDebInfo>:-O2 -g>
  $<$<CONFIG:MinSizeRel>:-Os>)
