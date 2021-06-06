// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* positionFragmentShaderSource = R"(
#version 330 core

// Access fragmet coordinates in integer steps
// TODO: explain more
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 color;

uniform sampler1D position_buffer;


void main() {
    int i = int(gl_FragCoord.x); // Index of the particle
    vec2 position = vec2(texelFetch(position_buffer, i, 0)); // previous position

    color = vec4(position.x+1, position.y, 0.0f, 1.0f);
}
)";
