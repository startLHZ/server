cmake_minimum_required(VERSION 3.0.0)

project(thread_pool)

include_directories(include)

add_executable(thread_pool src/main.cpp src/task_queue.cpp src/thread_pool.cpp src/server.cpp)

add_compile_options(-std=c++17 -pthread)