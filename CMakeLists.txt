cmake_minimum_required(VERSION 3.16)

project(monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

add_compile_options(-Wall -Wextra -O2)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(monitor monitor.cpp)

target_link_libraries(monitor PRIVATE Threads::Threads)
