cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

project(AntaEngine VERSION 1.0)

file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol/*.c") # for wayland

include_directories(includes)
add_executable(AntaEngine
    ${LIB_SOURCES}

    src/main.c
    src/utils.c
    src/engine.c
    src/vulkan_wrapper.c
    src/wayland.c # for wayland
)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED) # for wayland

pkg_check_modules(WAYLAND REQUIRED wayland-client) # for wayland

target_include_directories(AntaEngine PRIVATE
    ${WAYLAND_INCLUDE_DIRS} # for wayland
    "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol" # for wayland
)

target_link_libraries(AntaEngine PRIVATE
    Vulkan::Vulkan
    ${WAYLAND_LIBRARIES} # for wayland
)

# for wayland
target_compile_options(AntaEngine PRIVATE ${WAYLAND_CFLAGS_OTHER})
target_compile_definitions(AntaEngine PRIVATE WAYLAND_SURFACE)
