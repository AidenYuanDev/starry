include(FetchContent)

# 添加 Google Test
# if(BUILD_TESTING)
# FetchContent_Declare(
#   googletest
#   GIT_REPOSITORY https://github.com/google/googletest.git
#   GIT_TAG       main 
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# # gmock
# set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) 
# # gtest
# set(BUILD_GTEST ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)
# endif()

# 添加 protobuf 
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

# 使用 pkg-config 查找 abseil
find_package(PkgConfig REQUIRED)
pkg_check_modules(ABSL IMPORTED_TARGET absl_raw_hash_set absl_hash absl_log absl_log_internal_check_op)

# 添加 zlib
find_package(ZLIB)

# absl
find_package(absl REQUIRED)
