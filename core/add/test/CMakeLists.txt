enable_testing()

add_executable(unit_tests
    test_module1.cpp
    test_module2.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main
        module1
        module2
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
