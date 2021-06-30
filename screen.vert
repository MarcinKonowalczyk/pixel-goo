// Vertex shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* screenVertexShaderSource = R"(
#version 330 core

uniform vec2 window_size;
uniform sampler1D position_buffer;
uniform sampler1D velocity_buffer;

out float velocity;

void main() {
    vec2 position = vec2(texelFetch(position_buffer, gl_VertexID, 0));
    velocity = length(vec2(texelFetch(velocity_buffer, gl_VertexID, 0)));

    vec2 normalised_coordinates = vec2(
        position.x/(window_size.x/2)-1,
        -(position.y/(window_size.y/2)-1)
        );

    gl_Position = vec4(normalised_coordinates.x, normalised_coordinates.y, 0.0f, 1.0f);
    gl_PointSize = 2.0f;
}
)";