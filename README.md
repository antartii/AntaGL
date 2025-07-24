# AntaGL
AntaGL (Anta's Graphic Library) is a C library made as a hobby to use vulkan with different platform-specific surface APIs.
The original goal of this library is to be the base both the visual interface of a game engine and the visual interface of games users can create with engine.

We choosed to use C as library because of how we liked the language being as explicit as possible and because the vulkan library was created to compile on C99, but since a lot of peoples like to use the C++ languages we will create a wrapper around some function to create a AntaGL working on C++ in the future.

## Core features
- Vulkan and wayland support
- Camera view and projection support using cglm for transformations
- Objects creation supporting basic (triangles, rectangles) and complex shapes
- Precompiled shaders for vertex and fragmentation stages

## Dependencies
- A C compiler (gcc or clang)
- CMake (3.14 or later)
- Vulkan (1.3 or later)
- cglm (0.9.6 or later)
- [Only for wayland support] Wayland development packages

## Build
- Configure using cmake
    ```bash
    cmake -B build <OPTIONS> .
    ```
    Here is the list of the available options:
    |options|values|description|
    |-|-|-|
    |`DSURFACE`|`wayland`|The surface used by the engine, `wayland` is the default|
    |`DCMAKE_BUILD_TYPE`|`None`|Activate the `#define DEBUG` flag while compiling source and includes files, `None` is the default|

- Build the project
    ```bash
    cmake --build build
    ```
- Install the library with admin privilege
    ```bash
    cmake --install build
    ```

## Compiling with AntaGL
- Find the package installed
```bash
find_package(AntaGL REQUIRED)
```
- Link the library to your target
```bash
target_link_libraries(${MAIN_TARGET} PUBLIC AntaGL::AntaGL)
```
