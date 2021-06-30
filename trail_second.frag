// Fragment shader source
// This file will be #included in the source at compile time
// The actuall source must therefore be in an R-string
const GLchar* trailSecondFragmentShaderSource = R"(
#version 330 core
uniform float trail_intensity;

uniform sampler1D velocity_buffer;

out vec4 color;

in float VertexID; // 'in' cant be an int

#define ISQRT2 0.7071067811865476 // COS PI/4
#define COSPID8 0.9238795325112867 // COS PI/8
#define COSPID16 0.9807852804032304 // COS PI/16
#define PI 3.141592653589793

void main() {
    // Discard each point outside of the circle radius
    // https://stackoverflow.com/a/27099691/2531987
    vec2 circCoord = 2.0 * gl_PointCoord.xy - 1.0;
    if (dot(circCoord, circCoord) > 1.0) { discard; }

    vec2 velocity = vec2(texelFetch(velocity_buffer, int(VertexID), 0));

    float velocity_magnitude = length(velocity);

    vec2 velocity_normal = velocity / velocity_magnitude;
    // if (velocity_magnitude == 0) {discard;}
    if (velocity_magnitude < 1.0) {discard;}

    float r = circCoord.x*circCoord.x + circCoord.y*circCoord.y;
    float theta = dot(circCoord.xy,velocity_normal)/length(circCoord);
    if (theta > -COSPID16) {discard;}

    color = vec4(1.0f, 0.0f, 0.0f, trail_intensity);    
}

)";
