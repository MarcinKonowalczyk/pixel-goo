// Vertex shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* screenVertexShaderSource = R"(
#version 330 core

uniform vec2 window_size;
uniform sampler1D position_buffer;
uniform sampler1D velocity_buffer;

out float velocity;

vec2 screenNormalisedCoords(vec2 coordinate) {
    coordinate = mod(coordinate, window_size);
    return vec2(
        +coordinate.x/(window_size.x/2)-1,
        -(coordinate.y/(window_size.y/2)-1)
        );
}

void main() {
    vec2 position = vec2(texelFetch(position_buffer, gl_VertexID, 0));
    velocity = length(vec2(texelFetch(velocity_buffer, gl_VertexID, 0)));

    gl_Position = vec4(screenNormalisedCoords(position), 0.0f, 1.0f);
    gl_PointSize = 2.0f;
}
)";