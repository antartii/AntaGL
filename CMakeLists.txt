cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

project(AntaEngine VERSION 1.0)

add_executable(AntaEngine
    src/main.c
)

find_package(Vulkan)
target_link_libraries(AntaEngine PRIVATE Vulkan::Vulkan)
