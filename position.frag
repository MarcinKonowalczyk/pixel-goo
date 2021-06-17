// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* positionFragmentShaderSource = R"(
#version 330 core

// Access fragmet coordinates in integer steps
// TODO: explain more
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 out_position;

in float VertexID;

uniform vec2 window_size;
uniform sampler1D position_buffer;
uniform sampler1D velocity_buffer;

void main() {
    int i = int(gl_FragCoord.x); // Index of the particle
    vec2 position = vec2(texelFetch(position_buffer, i, 0)); // previous position
    vec2 velocity = vec2(texelFetch(velocity_buffer, i, 0)); // velocity
    vec2 delta_position = velocity;
    // delta_position += random(vec2(0,0) + VertexID + epoch_counter);
    // delta_position += +vec2(1.0, 1.0); // drift

    vec2 new_position = position +  delta_position;
    new_position = mod(new_position, window_size);

    out_position = vec4(new_position, 0.0, 1.0);
}
)";
