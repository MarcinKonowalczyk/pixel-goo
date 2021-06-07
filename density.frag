// Fragment shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* densityFragmentShaderSource = R"(
#version 330 core
out vec4 color;
uniform float density_alpha;

void main() {
    color = vec4(1.0f, 0.0f, 0.0f, density_alpha);
}
)";
