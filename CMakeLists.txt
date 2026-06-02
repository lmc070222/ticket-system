cmake_minimum_required(VERSION 3.10)
project(TicketSystem)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启优化选项（OJ 通常需要）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# 添加可执行文件，名字必须是 'code'
# 入口文件在 src/main.cpp
add_executable(code src/main.cpp)