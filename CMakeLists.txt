cmake_minimum_required(VERSION 3.15)
project(ArgParser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(examples)
