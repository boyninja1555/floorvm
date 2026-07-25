cmake_minimum_required(VERSION 4.2)
project(FloorVM LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# SDL3
find_package(SDL3 REQUIRED CONFIG)

# Executable
file(GLOB FV_SOURCES CONFIGURE_DEPENDS "src/*.c")
add_executable(FloorVM ${FV_SOURCES})
target_include_directories(FloorVM PRIVATE src include)
target_link_libraries(FloorVM PRIVATE SDL3::SDL3)