cmake_minimum_required(VERSION 3.14.7)

project(AdvancedImageProcessor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)

set(TARGET_NAME "AdvancedImageProcessor")

add_executable(${TARGET_NAME}
    src/main.cpp
)

target_link_libraries(${TARGET_NAME}
    -static -static-libgcc -static-libstdc++
    OpenMP::OpenMP_CXX gomp
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET_NAME} PRIVATE -Ofast)
endif()
