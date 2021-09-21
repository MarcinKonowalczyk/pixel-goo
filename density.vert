#version 330 core

uniform vec2 window_size;
uniform vec2 physics_buffer_size;
uniform sampler2D position_buffer;
uniform int density_map_downsampling;
uniform float kernel_radius;

void main() {
    ivec2 buffer_position = ivec2(gl_VertexID % int(physics_buffer_size.x), gl_VertexID / int(physics_buffer_size.y));
    vec2 position = vec2(texelFetch(position_buffer, buffer_position, 0));
    vec2 normalised_coordinates = vec2(
        position.x/(window_size.x/2)-1,
        -(position.y/(window_size.y/2)-1)
        );

    gl_Position = vec4(normalised_coordinates.x, normalised_coordinates.y, 0.0f, 1.0f);
    gl_PointSize = kernel_radius/density_map_downsampling;
}