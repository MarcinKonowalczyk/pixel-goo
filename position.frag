// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* positionFragmentShaderSource = R"(
#version 330 core

// Access fragmet coordinates in integer steps
// TODO: explain more
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 out_position;

uniform float window_width;
uniform float window_height;
uniform int epoch_counter;
uniform sampler1D position_buffer;

// Based on:
// https://thebookofshaders.com/10/
// http://patriciogonzalezvivo.com
vec2 random (vec2 seed) {
    float a = dot(seed.xy,vec2(0.890,0.870));
    float b = dot(seed.xy,vec2(-0.670,0.570));
    return 2*fract(sin(vec2(a,b))*43758.5453123)-1;
}

void main() {
    int i = int(gl_FragCoord.x); // Index of the particle
    vec2 position = vec2(texelFetch(position_buffer, i, 0)); // previous position
    vec2 delta_position = vec2(0,0);
    // delta_position += random(position.xy + epoch_counter); // diffusion
    delta_position += -vec2(1.0, 1.0); // drift

    vec2 new_position = position.xy+delta_position.xy;
    new_position = mod(new_position, vec2(window_width, window_height));

    out_position = vec4(new_position.xy, 0.0, 1.0);
}
)";
