cmake_minimum_required(VERSION 3.10)
project(TicketSystem)

# 1. 设置 C++ 17 标准（满足大部分现代 C++ 特性）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 包含头文件目录（让 main.cpp 能找到 include/ 下的类定义）
include_directories(include)

# 3. 开启 O2 编译优化选项，这对于压力测试点不 TLE 至关重要
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# 4. 收集 src 目录下的所有源文件（如果你的 parser.cpp, trainmanager.cpp 等在 src 下）
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 5. 添加可执行文件，名字按照大作业评测要求指定为 'code'
add_executable(code ${SOURCES})