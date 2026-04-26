add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../googletest-1.17.0 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)

# 黑盒测试
add_executable(test_black_box test_black_box.cpp)
target_link_libraries(test_black_box PRIVATE gtest_main)
target_include_directories(test_black_box PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_black_box PRIVATE TESTING)
target_compile_options(test_black_box PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(test_black_box PRIVATE -fprofile-arcs -ftest-coverage)

# 白盒测试
add_executable(test_white_box test_white_box.cpp)
target_link_libraries(test_white_box PRIVATE gtest_main)
target_include_directories(test_white_box PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_white_box PRIVATE TESTING)
target_compile_options(test_white_box PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(test_white_box PRIVATE -fprofile-arcs -ftest-coverage)

add_test(NAME BlackBoxTest COMMAND test_black_box)
add_test(NAME WhiteBoxTest COMMAND test_white_box)