cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaGLExample_Basic)
project(${MAIN_TARGET} VERSION 1.0.0)

find_package(AntaGL REQUIRED)

add_executable(${MAIN_TARGET} ${PROJECT_SOURCE_DIR}/src/main.c)

set(SURFACE "wayland" CACHE STRING "Select your surface")

if (SURFACE STREQUAL "win32")
    target_link_options(AntaGLExample_Basic PRIVATE "/SUBSYSTEM:WINDOWS")
endif()

target_include_directories(${MAIN_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_link_libraries(${MAIN_TARGET} PRIVATE AntaGL::AntaGL)
