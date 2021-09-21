#!/usr/bin/env python3

# Helper script to generate 'add_custom_command' includes for the glsl shader source includes

HEADER = """

###################################################################
# Automatically generated block for inclusion of the shader files #
# See "generate_cmake_includes.py" for more information           #
###################################################################

# Set shaders directory and create it at build generation time
set(SHADERS_DIR @{h_dir}@)
file(MAKE_DIRECTORY ${SHADERS_DIR})
"""

BODY = """
add_custom_command(
    OUTPUT ${SHADERS_DIR}/@{program}@.h
    COMMAND python3 @{embed_shader_py_dir}@ @{shader_sources}@ ${SHADERS_DIR}/@{program}@.h
    DEPENDS @{embed_shader_py_dir}@ @{shader_sources}@
)
add_custom_target( generate_@{program}@ DEPENDS ${SHADERS_DIR}/@{program}@.h )
add_dependencies( @{main_target_name}@ generate_@{program}@ )
"""

FOOTER = """
target_include_directories( @{main_target_name}@ PUBLIC ${SHADERS_DIR} )

###################################################################
# End of automatically generated block                            #
###################################################################

"""

main_target_name = "goo"
embed_shader_py_dir = "${CMAKE_CURRENT_SOURCE_DIR}/embed_shader.py"
shaders_dir = "${CMAKE_CURRENT_SOURCE_DIR}"
h_dir = "${CMAKE_CURRENT_BINARY_DIR}/shaders"

# https://stackoverflow.com/a/26531467/2531987
glsl_pipeline_order = {
    ".vert": 0,  # Vartex shader
    ".tasc": 1,  # Tasselation control shader
    ".tase": 2,  # Tasselation evaluation shader
    ".geom": 3,  # Geometry shader
    ".frag": 4,  # Fragment shader
    ".comp": 5,  # Compute shader
}

import os
from collections import defaultdict

programs = defaultdict(list)


# Find all shaders for each program
for file in os.listdir("."):
    name, ext = os.path.splitext(file)
    if ext in glsl_pipeline_order:
        programs[name].append(ext)


# Sort in pipeline order
for program in programs:
    programs[program].sort(key=lambda val: glsl_pipeline_order[val])


# Create all sources
sources = []
for program, shaders in programs.items():
    source = ""
    for shader in shaders:
        source += f"{shaders_dir}/{program}{shader} "
    source = source[:-1]
    sources.append((program, source))


# Create cmake shader includes

text = ""

header = HEADER
header = header.replace("@{h_dir}@", h_dir)
text += header

for program, source in sources:

    body = BODY
    body = body.replace("@{main_target_name}@", main_target_name)
    body = body.replace("@{embed_shader_py_dir}@", embed_shader_py_dir)
    body = body.replace("@{program}@", program)
    body = body.replace("@{shader_sources}@", source)
    text += body

footer = FOOTER
footer = footer.replace("@{main_target_name}@", main_target_name)
text += footer

assert not any(l == "@" for l in text)

import sys

argv = sys.argv
if len(argv) == 2:
    output_filename = argv[1]
    with open(output_filename, "w") as f:
        f.write(text)
else:
    print(text)
