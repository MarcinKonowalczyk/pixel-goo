// Vertex shader source
// This file will be #included in the source at compile time
// The actuall rource must therefore be in an R-string
const GLchar* screenVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;

uniform float window_width;
uniform float window_height;
uniform sampler1D position_map;

void main() {
    vec3 temp = vec3(texelFetch(position_map, gl_VertexID, 0));
    // vec2 normalised_coordinates = vec2(
    //     position.x/(window_width/2)-1,
    //     -(position.y/(window_height/2)-1)
    //     );
    vec2 normalised_coordinates = vec2(
        temp.x/(window_width/2)-1,
        -(temp.y/(window_height/2)-1)
        );

    gl_Position = vec4(normalised_coordinates.x, normalised_coordinates.y, 0.0f, 1.0f);
    gl_PointSize = 2.0f;
}
)";