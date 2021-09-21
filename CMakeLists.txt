cmake_minimum_required(VERSION 3.10)
project(goo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-Wfatal-errors")
# set(CMAKE_CXX_FLAGS "-O3 -Wfatal-errors")
# set(CMAKE_C_FLAGS "-O3 -Wfatal-errors")

add_subdirectory( lib/glad )

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory( lib/glfw )

add_subdirectory( lib/glm )

set( goo-SRC main.cpp )
add_executable( goo ${goo-SRC} )

target_include_directories(goo PUBLIC
    "${PROJECT_BINARY_DIR}"
    lib/stb
    )

target_link_libraries( goo glad glfw glm )