// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* positionFragmentShaderSource = R"(
#version 330 core
out vec2 new_position;

// uniform sampler2D density_map;
// uniform float window_width;
// uniform float window_height;

in vec2 new_coordinates;

void main() {
    // vec2 temp = vec2(gl_FragCoord.x/window_width, -gl_FragCoord.y/window_height);
    // color = vec4(new_coordinates.x, new_coordinates.y, 0.0, 1.0);
    new_position = vec2(100.0f, 100.0f);
    // color = vec4(0.067f, 0.455f, 0.729f, 1.0f);
}
)";
