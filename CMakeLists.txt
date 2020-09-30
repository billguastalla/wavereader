# ./

cmake_minimum_required(VERSION 3.0)
project(WAVEIO LANGUAGES CXX VERSION 0.1)

add_subdirectory(src) 
enable_testing()
add_subdirectory(test)
