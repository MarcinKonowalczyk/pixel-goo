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

set(SOURCES
    main.cpp
    shader.cpp
    physics_buffer.cpp
)

add_executable( goo ${SOURCES} )

target_include_directories(goo PUBLIC
    "${PROJECT_BINARY_DIR}"
    # lib/stb
    )

target_link_libraries( goo PRIVATE glad glfw glm )


###################################################################
# Automatically generated block for inclusion of the shader files #
# See "generate_cmake_includes.py" for more information           #
###################################################################

# Set shaders directory and create it at build generation time
set(SHADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADERS_DIR})

add_custom_command(
    OUTPUT ${SHADERS_DIR}/copy.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/copy.vert ${CMAKE_CURRENT_SOURCE_DIR}/copy.frag ${SHADERS_DIR}/copy.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/copy.vert ${CMAKE_CURRENT_SOURCE_DIR}/copy.frag
)
add_custom_target( generate_copy DEPENDS ${SHADERS_DIR}/copy.h )
add_dependencies( goo generate_copy )

add_custom_command(
    OUTPUT ${SHADERS_DIR}/density.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/density.vert ${CMAKE_CURRENT_SOURCE_DIR}/density.frag ${SHADERS_DIR}/density.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/density.vert ${CMAKE_CURRENT_SOURCE_DIR}/density.frag
)
add_custom_target( generate_density DEPENDS ${SHADERS_DIR}/density.h )
add_dependencies( goo generate_density )

add_custom_command(
    OUTPUT ${SHADERS_DIR}/position.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/position.vert ${CMAKE_CURRENT_SOURCE_DIR}/position.frag ${SHADERS_DIR}/position.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/position.vert ${CMAKE_CURRENT_SOURCE_DIR}/position.frag
)
add_custom_target( generate_position DEPENDS ${SHADERS_DIR}/position.h )
add_dependencies( goo generate_position )

add_custom_command(
    OUTPUT ${SHADERS_DIR}/screen.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/screen.vert ${CMAKE_CURRENT_SOURCE_DIR}/screen.frag ${SHADERS_DIR}/screen.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/screen.vert ${CMAKE_CURRENT_SOURCE_DIR}/screen.frag
)
add_custom_target( generate_screen DEPENDS ${SHADERS_DIR}/screen.h )
add_dependencies( goo generate_screen )

add_custom_command(
    OUTPUT ${SHADERS_DIR}/trail.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/trail.vert ${CMAKE_CURRENT_SOURCE_DIR}/trail.frag ${SHADERS_DIR}/trail.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/trail.vert ${CMAKE_CURRENT_SOURCE_DIR}/trail.frag
)
add_custom_target( generate_trail DEPENDS ${SHADERS_DIR}/trail.h )
add_dependencies( goo generate_trail )

add_custom_command(
    OUTPUT ${SHADERS_DIR}/velocity.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/velocity.vert ${CMAKE_CURRENT_SOURCE_DIR}/velocity.frag ${SHADERS_DIR}/velocity.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/velocity.vert ${CMAKE_CURRENT_SOURCE_DIR}/velocity.frag
)
add_custom_target( generate_velocity DEPENDS ${SHADERS_DIR}/velocity.h )
add_dependencies( goo generate_velocity )

target_include_directories( goo PUBLIC ${SHADERS_DIR} )

###################################################################
# End of automatically generated block                            #
###################################################################


