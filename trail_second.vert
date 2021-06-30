// Vertex shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* trailSecondVertexShaderSource = R"(
#version 330 core

// uniform sampler2D trail_map;

uniform vec2 window_size;
uniform int trail_map_downsampling;
uniform float kernel_radius;

uniform sampler1D position_buffer;
uniform sampler1D velocity_buffer;

out vec2 velocity;

// out float previous_trail;
// vec2 textureNormalisedCoordinates(vec2 coordinate) {
//     coordinate = mod(coordinate, window_size);
//     return vec2(+coordinate.x/window_size.x, -(coordinate.y/window_size.y));
// }

vec2 screenNormalisedCoordinates(vec2 coordinate) {
    coordinate = mod(coordinate, window_size);
    return vec2(+coordinate.x/(window_size.x/2)-1, -(coordinate.y/(window_size.y/2)-1));
}

void main() {
    vec2 position = vec2(texelFetch(position_buffer, gl_VertexID, 0));
    velocity = vec2(texelFetch(velocity_buffer, gl_VertexID, 0));

    gl_Position = vec4(screenNormalisedCoordinates(position), 0.0f, 1.0f);
    gl_PointSize = kernel_radius/trail_map_downsampling;
    // gl_PointSize = 10.0f;
}

)";