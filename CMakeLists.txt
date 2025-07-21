cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaEngine)

project(&{MAIN_TARGET} VERSION 1.0)

option(ENABLE_WAYLAND "Enable Wayland support" OFF)

if (ENABLE_WAYLAND)
    file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol/*.c")
    include_directories(includes/wayland)
else()
    set(LIB_SOURCES)
endif()

include_directories(includes
    includes/vulkan
)
add_executable(${MAIN_TARGET}
    ${LIB_SOURCES}

    src/main.c
    src/utils.c
    src/engine.c
    src/vertex.c
    src/model.c
    src/vulkan/vulkan_wrapper.c
    src/vulkan/vulkan_extension_wrapper.c
    $<$<BOOL:${ENABLE_WAYLAND}>:${CMAKE_SOURCE_DIR}/src/wayland/wayland.c>
)

find_package(Vulkan REQUIRED)
target_link_libraries(${MAIN_TARGET} PRIVATE Vulkan::Vulkan)

if (ENABLE_WAYLAND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED wayland-client)

    target_include_directories(${MAIN_TARGET} PRIVATE
        ${WAYLAND_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/lib/xdg_shell_protocol"
    )

    target_link_libraries(${MAIN_TARGET} PRIVATE ${WAYLAND_LIBRARIES})
    target_compile_options(${MAIN_TARGET} PRIVATE ${WAYLAND_CFLAGS_OTHER})
    target_compile_definitions(${MAIN_TARGET} PRIVATE WAYLAND_SURFACE)
endif()

target_compile_definitions(${MAIN_TARGET} PRIVATE DEBUG)

function(add_slang_shader_target TARGET)
    cmake_parse_arguments(SHADER "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(SHADERS_BUILD_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")
    else()
        set(SHADERS_BUILD_DIR "${CMAKE_BINARY_DIR}/shaders")
    endif()
    set(SLANG_OUTPUT ${SHADERS_BUILD_DIR}/slang.spv)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)

    file(MAKE_DIRECTORY ${SHADERS_BUILD_DIR})

    if(NOT SLANGC_EXECUTABLE)
        set(SLANGC_EXECUTABLE $ENV{SLANGC_EXECUTABLE})
    endif()

    add_custom_command(
        OUTPUT ${SLANG_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_BUILD_DIR}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${SLANG_OUTPUT}
        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADER_SOURCES}
        COMMENT "Compiling Slang shaders"
        VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SLANG_OUTPUT})
endfunction()

add_slang_shader_target(SlangShader SOURCES ${CMAKE_SOURCE_DIR}/shaders/shader.slang)
add_dependencies(${MAIN_TARGET} SlangShader)
