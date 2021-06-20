// Fragment shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* trailFragmentShaderSource = R"(
#version 330 core
out vec4 color;
uniform float trail_alpha;

uniform sampler1D position_buffer;
uniform sampler1D velocity_buffer;
uniform sampler2D trail_map;

in float VertexID; // 'in' cant be an int

#define ISQRT2 0.7071067811865476 // COS PI/4
#define COSPID8 0.9238795325112867 // COS PI/8
#define PI 3.141592653589793

void main() {
    // Discard each point outside of the circle radius
    // https://stackoverflow.com/a/27099691/2531987
    vec2 circCoord = 2.0 * gl_PointCoord.xy - 1.0;
    if (dot(circCoord, circCoord) > 1.0) { discard; }

    vec2 position = vec2(texelFetch(position_buffer, int(VertexID), 0));
    vec2 velocity = vec2(texelFetch(velocity_buffer, int(VertexID), 0));
    float velocity_magnitude = length(velocity);

    vec2 velocity_normal = velocity / velocity_magnitude;

    // if (velocity_magnitude > 0) {
    //     vec2 velocity_normal = velocity / velocity_magnitude;
    //     float velocityDot = dot(velocity_normal, circCoord);
    //     // if (velocityDot > 0) { discard; }
    //     if (velocityDot > 0.5) { discard; }
    //     // if (velocityDot < -0.1) { discard; }
    // }

    float r = circCoord.x*circCoord.x + circCoord.y*circCoord.y;
    float theta = dot(circCoord.xy,velocity_normal)/length(circCoord);
    if (theta > -COSPID8) {discard;}
    color = vec4(1.0f, 0.0f, 0.0f, trail_alpha);
}

)";
