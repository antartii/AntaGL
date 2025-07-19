cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaEngine)

project(&{MAIN_TARGET} VERSION 1.0)

file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol/*.c") # for wayland

include_directories(includes)
add_executable(${MAIN_TARGET}
    ${LIB_SOURCES}

    src/main.c
    src/utils.c
    src/engine.c
    src/vulkan_wrapper.c
    src/vulkan_extension_wrapper.c
    src/wayland.c # for wayland
)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED) # for wayland

pkg_check_modules(WAYLAND REQUIRED wayland-client) # for wayland

target_include_directories(${MAIN_TARGET} PRIVATE
    ${WAYLAND_INCLUDE_DIRS} # for wayland
    "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol" # for wayland
)

target_link_libraries(${MAIN_TARGET} PRIVATE
    Vulkan::Vulkan
    ${WAYLAND_LIBRARIES} # for wayland
)

# for wayland
target_compile_options(${MAIN_TARGET} PRIVATE ${WAYLAND_CFLAGS_OTHER})
target_compile_definitions(${MAIN_TARGET} PRIVATE
    WAYLAND_SURFACE
    DEBUG)

# for compiling shaders using slang
function(add_slang_shader_target TARGET)
    cmake_parse_arguments(SHADER "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
    set(SLANG_OUTPUT ${SHADERS_DIR}/slang.spv)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)

    file(MAKE_DIRECTORY ${SHADERS_DIR})

    if(NOT SLANGC_EXECUTABLE)
        set(SLANGC_EXECUTABLE $ENV{SLANGC_EXECUTABLE})
    endif()

    message(STATUS "Slang command: ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv")

    add_custom_command(
        OUTPUT ${SLANG_OUTPUT}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv

        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADER_SOURCES}
        COMMENT "Compiling Slang Shaders"
        VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SLANG_OUTPUT})
endfunction()

add_slang_shader_target(SlangShader SOURCES ${CMAKE_SOURCE_DIR}/shaders/shader.slang)
add_dependencies(${MAIN_TARGET} SlangShader)
