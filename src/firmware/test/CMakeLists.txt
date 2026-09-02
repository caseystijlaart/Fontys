find_package(GTest REQUIRED)

add_executable(sensing_tests test_example.cpp)
target_link_libraries(sensing_tests PRIVATE sensing_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(sensing_tests)
