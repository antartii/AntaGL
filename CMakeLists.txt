# === SETUP ===
cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaGL)
set(TARGET_VERSION 0.1.0)
project(${MAIN_TARGET} VERSION ${TARGET_VERSION})

# === OPTIONS ===
set(SURFACE "wayland" CACHE STRING "Select your surface")

# === SURFACE SELECTION ===
if (SURFACE STREQUAL "wayland")
    file(GLOB_RECURSE LIB_SOURCES ${PROJECT_SOURCE_DIR}/lib/xdg_shell_protocol/*.c)
    set(SURFACE_SOURCES ${PROJECT_SOURCE_DIR}/src/wayland/wayland.c)
    set(SURFACE_INCLUDES_DIR ${PROJECT_SOURCE_DIR}/includes/wayland
        ${PROJECT_SOURCE_DIR}/lib/xdg_shell_protocol
    )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED wayland-client)
    set(SURFACE_LIBS ${WAYLAND_LIBRARIES})
    set(SURFACE_CFLAGS ${WAYLAND_CFLAGS_OTHER})
    set(SURFACE_CDEF WAYLAND_SURFACE)
else()
    set(LIB_SOURCES)
    set(SURFACE_SOURCES)
    set(SURFACE_INCLUDES_DIR)
    set(SURFACE_LIBS)
    set(SURFACE_CFLAGS)
    set(SURFACE_CDEF)
endif()

# === CREATING THE TARGET
add_library(${MAIN_TARGET}
    ${LIB_SOURCES}
    ${SURFACE_SOURCES}
    
    ${PROJECT_SOURCE_DIR}/src/utils.c
    ${PROJECT_SOURCE_DIR}/src/engine.c
    ${PROJECT_SOURCE_DIR}/src/vertex.c
    ${PROJECT_SOURCE_DIR}/src/object.c
    ${PROJECT_SOURCE_DIR}/src/camera.c
    ${PROJECT_SOURCE_DIR}/src/vulkan/vulkan_wrapper.c
    ${PROJECT_SOURCE_DIR}/src/vulkan/vulkan_extension_wrapper.c
)
target_include_directories(${MAIN_TARGET} PUBLIC
    $<BUILD_INTERFACE:${SURFACE_INCLUDES_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes/vulkan>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/xdg_shell_protocol>
    $<INSTALL_INTERFACE:include/${MAIN_TARGET}>
)
target_link_libraries(${MAIN_TARGET} PUBLIC ${SURFACE_LIBS})
target_compile_options(${MAIN_TARGET} PUBLIC ${SURFACE_CFLAGS})
target_compile_definitions(${MAIN_TARGET} PUBLIC ${SURFACE_CDEF})
find_package(Vulkan REQUIRED)
target_link_libraries(${MAIN_TARGET} PUBLIC Vulkan::Vulkan m)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${MAIN_TARGET} PUBLIC DEBUG)
endif()

# === INSTALL THE TARGEST ==
install(TARGETS ${MAIN_TARGET}
    EXPORT ${MAIN_TARGET}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY includes/ DESTINATION include/${MAIN_TARGET}/)
install(DIRECTORY lib/xdg_shell_protocol/ DESTINATION include/${MAIN_TARGET}/ FILES_MATCHING PATTERN *.h)
set(SHADERS_BUILD_DIR "${CMAKE_BINARY_DIR}/shaders")
set(SLANG_OUTPUT ${SHADERS_BUILD_DIR}/slang.spv)
install(FILES ${SLANG_OUTPUT} DESTINATION share/${MAIN_TARGET}/shaders)
include(CMakePackageConfigHelpers)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET}ConfigVersion.cmake"
    VERSION ${TARGET_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file("${CMAKE_SOURCE_DIR}/${MAIN_TARGET}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${MAIN_TARGET}
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET}ConfigVersion.cmake"
    DESTINATION lib/cmake/${MAIN_TARGET}
)
install(EXPORT ${MAIN_TARGET}Targets
    FILE ${MAIN_TARGET}Targets.cmake
    NAMESPACE ${MAIN_TARGET}::
    DESTINATION lib/cmake/${MAIN_TARGET}
)

# === COMPILING SHADERS ===
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

add_slang_shader_target(SlangShader SOURCES ${PROJECT_SOURCE_DIR}/shaders/shader.slang)
add_dependencies(${MAIN_TARGET} SlangShader)
