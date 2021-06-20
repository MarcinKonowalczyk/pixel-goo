// Vertex shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* trailVertexShaderSource = R"(
#version 330 core

uniform vec2 window_size;
uniform sampler1D position_buffer;
uniform int trail_map_downsampling;
uniform float kernel_radius;

out float VertexID;

void main() {
    vec2 position = vec2(texelFetch(position_buffer, gl_VertexID, 0));
    vec2 normalised_coordinates = vec2(
        position.x/(window_size.x/2)-1,
        -(position.y/(window_size.y/2)-1)
        );

    gl_Position = vec4(normalised_coordinates.x, normalised_coordinates.y, 0.0f, 1.0f);
    gl_PointSize = kernel_radius/trail_map_downsampling;
    // gl_PointSize = 10.0f;

    VertexID = gl_VertexID;
}

)";