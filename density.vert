// Vertex shader source
// This file will be #included in the source at compile time
// The actuall rource must therefore be in an R-string
const GLchar* densityVertexShaderSource = R"(
#version 330 core

uniform float window_width;
uniform float window_height;
uniform sampler1D position_map;

void main() {
    vec2 temp = vec2(texelFetch(position_map, gl_VertexID, 0));
    vec2 normalised_coordinates = vec2(
        temp.x/(window_width/2)-1,
        -(temp.y/(window_height/2)-1)
        );

    gl_Position = vec4(normalised_coordinates.x, normalised_coordinates.y, 0.0f, 1.0f);
    gl_PointSize = 40.0f;
}
)";