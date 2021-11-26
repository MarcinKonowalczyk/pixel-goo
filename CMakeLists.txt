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
    buffer.cpp
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
set(cmake_shaders_dir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${cmake_shaders_dir})

add_custom_command(
    OUTPUT ${cmake_shaders_dir}/copy.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/copy.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/copy.frag ${cmake_shaders_dir}/copy.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/copy.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/copy.frag
)
add_custom_target( generate_copy DEPENDS ${cmake_shaders_dir}/copy.h )
add_dependencies( goo generate_copy )

add_custom_command(
    OUTPUT ${cmake_shaders_dir}/density.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/density.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/density.frag ${cmake_shaders_dir}/density.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/density.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/density.frag
)
add_custom_target( generate_density DEPENDS ${cmake_shaders_dir}/density.h )
add_dependencies( goo generate_density )

add_custom_command(
    OUTPUT ${cmake_shaders_dir}/position.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/position.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/position.frag ${cmake_shaders_dir}/position.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/position.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/position.frag
)
add_custom_target( generate_position DEPENDS ${cmake_shaders_dir}/position.h )
add_dependencies( goo generate_position )

add_custom_command(
    OUTPUT ${cmake_shaders_dir}/screen.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/screen.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/screen.frag ${cmake_shaders_dir}/screen.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/screen.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/screen.frag
)
add_custom_target( generate_screen DEPENDS ${cmake_shaders_dir}/screen.h )
add_dependencies( goo generate_screen )

add_custom_command(
    OUTPUT ${cmake_shaders_dir}/trail.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/trail.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/trail.frag ${cmake_shaders_dir}/trail.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/trail.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/trail.frag
)
add_custom_target( generate_trail DEPENDS ${cmake_shaders_dir}/trail.h )
add_dependencies( goo generate_trail )

add_custom_command(
    OUTPUT ${cmake_shaders_dir}/velocity.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/velocity.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/velocity.frag ${cmake_shaders_dir}/velocity.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/velocity.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/velocity.frag
)
add_custom_target( generate_velocity DEPENDS ${cmake_shaders_dir}/velocity.h )
add_dependencies( goo generate_velocity )

target_include_directories( goo PUBLIC ${cmake_shaders_dir} )

###################################################################
# End of automatically generated block                            #
###################################################################


