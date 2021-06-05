// Fragment shader source
// This file will be #included in the source at compile time
// The actuall rource must therefore be in an R-string
const GLchar* densityFragmentShaderSource = R"(
#version 330 core
out vec4 color;
void main() {
    color = vec4(1.0f, 1.0f, 1.0f, 0.1f);
}
)";
