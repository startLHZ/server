cmake_minimum_required(VERSION 3.0.0)

project(myserver)

# 添加编译选项
add_compile_options(-std=c++17 -pthread)

# 包含头文件目录
include_directories(include)
# include_directories(lib/spdlog)



# 创建可执行文件
add_executable(myserver 
    src/main.cpp 
    src/server.cpp
    src/logger.cpp
)

# 为目标添加包含路径
target_include_directories(myserver PUBLIC lib/spdlog)
target_include_directories(myserver PUBLIC lib)
