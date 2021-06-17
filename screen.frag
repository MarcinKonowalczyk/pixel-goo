// Fragment shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* screenFragmentShaderSource = R"(
#version 330 core
out vec4 color;

layout(pixel_center_integer) in vec4 gl_FragCoord;

uniform sampler2D density_map;
uniform int density_map_downsampling;
uniform vec2 window_size;

const vec4 color1 = vec4(0.067f, 0.455f, 0.729f, 1.0f);
// const vec4 color2 = vec4(0.843f, 0.329f, 0.149f, 1.0f);
const vec4 color2 = vec4(0.925f, 0.69f, 0.208f, 1.0f);

void main() {
    // indices of the particle

    vec2 position = gl_FragCoord.xy;
    vec2 normalised_position = vec2(
        +position.x/window_size.x,
        +(position.y/window_size.y)
        );
    float density = texture(density_map, normalised_position).x;

    // ivec2 position = ivec2(gl_FragCoord.xy/density_map_downsampling);
    // float density = texelFetch(density_map, position, 0).x;

    color = mix(color1, color2, density);
}
)";
