# === SETUP ===
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaGL)
set(TARGET_VERSION 0.1.0)
project(${MAIN_TARGET} VERSION ${TARGET_VERSION})

# === OPTIONS ===
set(SURFACE "wayland" CACHE STRING "Select your surface")

# === SURFACE SELECTION ===
if (SURFACE STREQUAL "wayland")
    file(GLOB_RECURSE LIB_SOURCES ${PROJECT_SOURCE_DIR}/lib/xdg/*.c)
    set(SURFACE_SOURCES ${PROJECT_SOURCE_DIR}/src/surfaces/wayland/wayland.c)
    set(SURFACE_INCLUDES_DIR ${PROJECT_SOURCE_DIR}/includes/surfaces/wayland
        ${PROJECT_SOURCE_DIR}/lib/xdg
    )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED wayland-client)
    set(SURFACE_LIBS ${WAYLAND_LIBRARIES})
    set(SURFACE_CFLAGS ${WAYLAND_CFLAGS_OTHER})
    set(SURFACE_CDEF WAYLAND_SURFACE)
elseif()
    set(LIB_SOURCES)
    set(SURFACE_SOURCES ${PROJECT_SOURCE_DIR}/src/windows_surface/windows_surface.c)
    set(SURFACE_INCLUDES_DIR ${PROJECT_SOURCE_DIR}/includes/windows_surface)
    set(SURFACE_LIBS)
    set(SURFACE_CFLAGS)
    set(SURFACE_CDEF)
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
    ${PROJECT_SOURCE_DIR}/src/surfaces/surface.c
    ${PROJECT_SOURCE_DIR}/src/vulkan/vulkan_wrapper.c
    ${PROJECT_SOURCE_DIR}/src/vulkan/vulkan_extension_wrapper.c
)
if (WIN32)
    set(INCLUDES_DESTINATION include/AntaGL)
    set(INSTALL_INTERFACE_INCLUDE include)
else()
    set(INCLUDES_DESTINATION include/AntaGL)
    set(INSTALL_INTERFACE_INCLUDE include/AntaGL)
endif()
target_include_directories(${MAIN_TARGET} PUBLIC
    $<BUILD_INTERFACE:${SURFACE_INCLUDES_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/>
    $<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDE}>
)
target_link_libraries(${MAIN_TARGET} PUBLIC ${SURFACE_LIBS})
target_compile_options(${MAIN_TARGET} PUBLIC ${SURFACE_CFLAGS})
target_compile_definitions(${MAIN_TARGET} PUBLIC ${SURFACE_CDEF})
find_package(Vulkan REQUIRED)
target_link_libraries(${MAIN_TARGET} PUBLIC Vulkan::Vulkan)
if (NOT WIN32)
    target_link_libraries(${MAIN_TARGET} PUBLIC m)
endif()
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
install(DIRECTORY ${PROJECT_SOURCE_DIR}/includes/ DESTINATION ${INCLUDES_DESTINATION} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY lib/xdg/ DESTINATION include/xdg FILES_MATCHING PATTERN *.h)
install(DIRECTORY lib/cglm/ DESTINATION include/cglm FILES_MATCHING PATTERN *.h)
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
