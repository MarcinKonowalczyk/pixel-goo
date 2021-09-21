#version 330 core

uniform vec2 window_size;
uniform vec2 physics_buffer_size;
uniform sampler2D position_buffer;
uniform sampler2D velocity_buffer;

out float velocity;

vec2 screenNormalisedCoords(vec2 coordinate) {
    coordinate = mod(coordinate, window_size);
    return vec2(
        +coordinate.x/(window_size.x/2)-1,
        -(coordinate.y/(window_size.y/2)-1)
        );
}

void main() {
    ivec2 buffer_position = ivec2(gl_VertexID % int(physics_buffer_size.x), gl_VertexID / int(physics_buffer_size.y));
    vec2 position = vec2(texelFetch(position_buffer, buffer_position, 0));
    velocity = length(vec2(texelFetch(velocity_buffer, buffer_position, 0)));

    gl_Position = vec4(screenNormalisedCoords(position), 0.0f, 1.0f);
    // gl_PointSize = 2.0f;
    gl_PointSize = 1.0f;
}