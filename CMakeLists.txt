cmake_minimum_required(VERSION 3.16)
project(WordGraphProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 必须启用测试
enable_testing()

# 普通可执行文件（正常运行时使用）
add_executable(wordgraph TextGraph.cpp)

# 添加测试子目录
add_subdirectory(tests)