// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* positionFragmentShaderSource = R"(
#version 330 core

// precision highp float;

// in vec2 new_coordinates;
out vec4 color;

// uniform sampler2D density_map;
// uniform float window_width;
// uniform float window_height;


void main() {
    // vec2 temp = vec2(gl_FragCoord.x/window_width, -gl_FragCoord.y/window_height);
    // color = vec4(new_coordinates.x, new_coordinates.y, 0.0, 1.0);
    color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    // color = vec4(0.067f, 0.455f, 0.729f, 1.0f);
}
)";
