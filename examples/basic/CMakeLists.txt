cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaGLExample_Basic)
project(${MAIN_TARGET} VERSION 1.0.0)

include_directories(includes)
add_executable(${MAIN_TARGET} ${PROJECT_SOURCE_DIR}/src/main.c)

find_package(AntaGL REQUIRED)

target_link_libraries(${MAIN_TARGET} PUBLIC AntaGL::AntaGL)
