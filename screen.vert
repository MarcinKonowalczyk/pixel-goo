// Vertex shader source
// This file will be #included in the source at compile time
// The actuall rource must therefore be in an R-string
const GLchar* screenVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;

uniform float window_width;
uniform float window_height;

// out vec3 outColor;
void main() {
    // Transform to normalised coordinates
    vec2 temp = vec2(
        position.x/(window_width/2)-1,
        -(position.y/(window_height/2)-1)
        );

    gl_Position = vec4(temp.x, temp.y, 0.0f, 1.0f);
    gl_PointSize = 2.0f;
}
)";