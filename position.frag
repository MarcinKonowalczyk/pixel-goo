// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* positionFragmentShaderSource = R"(
#version 330 core

// Access fragmet coordinates in integer steps
// TODO: explain more
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 new_position;

uniform sampler1D position_buffer;

// Based on:
// https://thebookofshaders.com/10/
// http://patriciogonzalezvivo.com
float random (float seed) {
    return 2*fract(sin(seed*12.9898)*43758.5453123)-1;
}

void main() {
    int i = int(gl_FragCoord.x); // Index of the particle
    vec2 position = vec2(texelFetch(position_buffer, i, 0)); // previous position

    vec2 delta_position = vec2(random(position.y), random(position.x));

    new_position = vec4(position.xy+delta_position.xy, 0.0f, 1.0f);
}
)";
