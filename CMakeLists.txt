
cmake_minimum_required(VERSION 3.10)

project(Snake)

add_executable(Snake src/main.cpp)

find_package(Threads REQUIRED)

target_link_libraries(Snake PRIVATE Threads::Threads)

