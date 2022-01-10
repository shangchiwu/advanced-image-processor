cmake_minimum_required(VERSION 3.14.7)

project(AdvancedImageProcessor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)

# third party libraries

# freetype
include_directories(${CMAKE_SOURCE_DIR}/libs/freetype/include/freetype2)
link_directories(${CMAKE_SOURCE_DIR}/libs/freetype/lib)

# GLFW
include_directories(${CMAKE_SOURCE_DIR}/libs/GLFW/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/GLFW/bin)

# ImGUI
include_directories(${CMAKE_SOURCE_DIR}/libs/ImGUI)
set(IMGUI_SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/imgui_impl_opengl2.cpp
    ${CMAKE_SOURCE_DIR}/libs/ImGUI/misc/freetype/imgui_freetype.cpp
)

# Native File Dialog Extended
include_directories(${CMAKE_SOURCE_DIR}/libs/nfd/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/nfd/bin)

# stb_image
include_directories(${CMAKE_SOURCE_DIR}/libs/stb)

# clip
set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples")
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests")
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/clip)
include_directories(${CMAKE_SOURCE_DIR}/libs/clip)

# font
include_directories(${CMAKE_SOURCE_DIR}/libs/font)

# main project

set(TARGET_NAME "AdvancedImageProcessor")

add_executable(${TARGET_NAME}
    ${IMGUI_SOURCE_FILE}

    src/main.cpp
    src/algorithms.cpp
    src/handlers.cpp
    src/image.cpp
    src/image_window.cpp
    src/stb_image_impl.cpp
    src/utility.cpp
)

target_link_libraries(${TARGET_NAME}
    -static -static-libgcc -static-libstdc++
    OpenGL::GL glfw3
    nfd
    freetype
    clip
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET_NAME} PRIVATE -Ofast)
endif()
