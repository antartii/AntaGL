# === SETUP ===
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(MAIN_TARGET AntaGLpp)
set(TARGET_VERSION 0.1.0)
project(${MAIN_TARGET} VERSION ${TARGET_VERSION})

set(INCLUDES_DIR_ANTAGL ${PROJECT_SOURCE_DIR}/../includes)

# === OPTIONS ===
set(SURFACE "wayland" CACHE STRING "Select your surface")
set(INSTALL_ANTAGL "yes" CACHE STRING "Do you want to also install the C library AntaGL (recommended if not installed beforehand)")

if(NOT (INSTALL_ANTAGL STREQUAL "yes"))
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
endif()

# === COMPILING THE C LIB ===
include(ExternalProject)

ExternalProject_Add(AntaGL
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/..
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -DSURFACE=${SURFACE}
    BUILD_ALWAYS YES
    INSTALL_COMMAND ${CMAKE_COMMAND} --install .
)

# === CREATING THE TARGET ===
add_library(${MAIN_TARGET}
    src/engine.cpp
    src/objects/object.cpp)

if (WIN32)
    set(INCLUDES_DESTINATION include/${MAIN_TARGET})
    set(INSTALL_INTERFACE_INCLUDE include)
else()
    set(INCLUDES_DESTINATION include/${MAIN_TARGET})
    set(INSTALL_INTERFACE_INCLUDE include/${MAIN_TARGET})
endif()

target_include_directories(${MAIN_TARGET} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes>
    $<BUILD_INTERFACE:${INCLUDES_DIR_ANTAGL}>
    $<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDE}>
)

# === INSTALL THE TARGET ==
install(TARGETS ${MAIN_TARGET}
    EXPORT ${MAIN_TARGET}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/includes/ DESTINATION ${INCLUDES_DESTINATION} FILES_MATCHING PATTERN "*.hpp")
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
