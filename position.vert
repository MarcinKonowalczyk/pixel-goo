// Vertex shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* positionVertexShaderSource = R"(
#version 330 core

// precision highp float;
// uniform float window_width;
// uniform float window_height;
// uniform sampler1D position_buffer;

// float rand(vec2 co){
//     return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
// }

// out vec2 new_coordinates;

const vec2[] corners = vec2[]( 
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0)
);

void main() {
    gl_Position = vec4(corners[gl_VertexID], 1.0, 1.0);
}

// void main() {
//     vec2 position = vec2(texelFetch(position_buffer, gl_VertexID, 0));
//     vec2 normalised_coordinates = vec2(
//         position.x/(window_width/2)-1,
//         -(position.y/(window_height/2)-1)
//         );

//     new_coordinates = normalised_coordinates + vec2(1.0, 0.0);

//     float N = float(textureSize(position_buffer, 0));
//     gl_Position = vec4(gl_VertexID/N, 0.0, 0.0f, 1.0f);
//     // gl_Position = vec4(0.5f, 0.0, 0.0, 1.0f);
//     // gl_PointSize = 1.0f;
// }
)";
