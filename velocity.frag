// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* velocityFragmentShaderSource = R"(
#version 330 core

// Access fragmet coordinates in integer steps
// TODO: explain more
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 out_velocity;

in float VertexID;

uniform int epoch_counter;
uniform sampler1D velocity_buffer;

// Based on:
// https://thebookofshaders.com/10/
// http://patriciogonzalezvivo.com
vec2 random (vec2 seed) {
    float a = dot(seed.xy,vec2(0.890,0.870));
    float b = dot(seed.xy,vec2(-0.670,0.570));
    return 2*fract(sin(vec2(a,b))*43758.5453123)-1;
}

void main() {
    int i = int(gl_FragCoord.x); // Index of the particle
    vec2 velocity = vec2(texelFetch(velocity_buffer, i, 0)); // previous velocity
    float velocity_magnitude = length(velocity);
    vec2 velocity_normal;
    if (velocity_magnitude == 0) {
        velocity_normal = random(vec2(1,0) + VertexID + epoch_counter);
    } else {
        velocity_normal = velocity / velocity_magnitude;
    }

    // Dither
    vec2 delta_velocity = vec2(0,0);
    delta_velocity += 1.0*random(vec2(0,0) + VertexID + epoch_counter);

    // Drag
    float drag_magnitude = min(velocity_magnitude, 1.55);
    delta_velocity -= drag_magnitude * velocity_normal;


    // delta_velocity += random(velocity + epoch_counter + VertexID); // diffusion
    // Drift
    delta_velocity += vec2(1.0, 1.0);

    vec2 new_velocity = velocity + delta_velocity;

    out_velocity = vec4(new_velocity, 0.0, 1.0);
}
)";
