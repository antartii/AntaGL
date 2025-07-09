cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

project(AntaEngine VERSION 1.0)

file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol/*.c")

include_directories(includes)
add_executable(AntaEngine
    ${LIB_SOURCES}

    src/main.c
    src/utils.c
    src/vulkan_wrapper.c
    src/wayland.c
)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND REQUIRED wayland-client)

target_include_directories(AntaEngine PRIVATE
    ${WAYLAND_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol"
)
target_link_libraries(AntaEngine PRIVATE
    Vulkan::Vulkan
    ${WAYLAND_LIBRARIES}
)

target_compile_options(AntaEngine PRIVATE ${WAYLAND_CFLAGS_OTHER})
