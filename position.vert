// Vertex shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* positionVertexShaderSource = R"(
#version 330 core

const vec2[] corners = vec2[](
    vec2(-1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(1.0, -1.0)
);

void main() {
    gl_Position = vec4(corners[gl_VertexID], 1.0, 1.0);
}

)";