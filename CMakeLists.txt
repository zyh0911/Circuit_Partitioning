set(PROJECT_NAME graph_part)
project(${PROJECT_NAME} LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.14)

add_subdirectory(src)


