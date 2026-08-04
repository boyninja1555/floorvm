cmake_minimum_required(VERSION 4.2)
project(fvmc LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Executable
add_subdirectory(libcfloor)
add_executable(fvmc src/main.c)
target_include_directories(fvmc PRIVATE include)
target_link_libraries(fvmc PRIVATE cfloor)