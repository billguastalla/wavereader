# ./

cmake_minimum_required(VERSION 3.11)
project(WAVEIO LANGUAGES CXX VERSION 0.1)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   set(THREADS_PREFER_PTHREAD_FLAG ON)
   find_package(Threads REQUIRED)
endif()

add_subdirectory(src) 
enable_testing()
add_subdirectory(test)
