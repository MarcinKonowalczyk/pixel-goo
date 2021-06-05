// Fragment shader source
// This file will be #included in the source at compile time
// The actuall rource must therefore be in an R-string
const GLchar* screenFragmentShaderSource = R"(
#version 330 core
out vec4 color;
uniform sampler2D density_map;

// ToDo: Use FramentPosition
uniform float window_width;
uniform float window_height;

void main() {
    vec2 temp = vec2(gl_FragCoord.x/window_width, -gl_FragCoord.y/window_height);
    color = texture(density_map, temp) * vec4(0.067f, 0.455f, 0.729f, 1.0f);
}
)";
