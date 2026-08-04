cmake_minimum_required(VERSION 4.2)
project(FloorVM LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# SDL3
find_package(SDL3 REQUIRED CONFIG)

# Executable
add_executable(FloorVM src/main.c src/util.c src/renderer.c src/input.c src/machine.c)
target_include_directories(FloorVM PRIVATE include)
target_link_libraries(FloorVM PRIVATE SDL3::SDL3)