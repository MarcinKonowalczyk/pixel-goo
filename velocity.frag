// Fragment shader source
// This file will be #included in the source at compile time
// The actuall srource must therefore be in an R-string
const GLchar* velocityFragmentShaderSource = R"(
#version 330 core

// Access fragmet coordinates in integer steps
// TODO: explain more
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 out_velocity;

uniform sampler2D density_map;
uniform int density_map_downsampling;
uniform float window_width;
uniform float window_height;

in float VertexID;

uniform int epoch_counter;
uniform float drag_coefficient;
uniform float dither_coefficient;
uniform sampler1D position_buffer;
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
    vec2 position = vec2(texelFetch(position_buffer, i, 0)); // current position
    vec2 velocity = vec2(texelFetch(velocity_buffer, i, 0)); // previous velocity

    // Sample density map
    vec2 normalised_position = vec2(
        +position.x/window_width,
        -(position.y/window_height)
        );
    float density = texture(density_map, normalised_position).x;

    // ivec2 density_map_position = ivec2(position/density_map_downsampling);
    // float density = texelFetch(density_map, density_map_position, 0).x;

    vec2 new_velocity = velocity;

    // Dither
    // new_velocity += dither_coefficient*random(vec2(0,0) + VertexID + epoch_counter);
    // new_velocity += (1-density) * dither_coefficient * random(vec2(0,0) + VertexID + epoch_counter);
    new_velocity += density * dither_coefficient * random(vec2(0,0) + VertexID + epoch_counter);


    // Drift
    // new_velocity += 0.1 * vec2(1.0, 1.0);
    // new_velocity += (1-density) * vec2(1.0, 1.0);
    new_velocity += density * vec2(3.0, 2.0);

    // Resolve drag after all other acceleration to make sure that very high drag coefficient works
    float velocity_magnitude = length(new_velocity);
    vec2 velocity_normal;
    if (velocity_magnitude == 0) { // Return random nomral if magnitude is zero
        velocity_normal = random(vec2(1,0) + VertexID + epoch_counter);
        velocity_normal /= length(velocity_normal);
    } else {
        velocity_normal = new_velocity / velocity_magnitude;
    }

    float drag_magnitude = drag_coefficient * velocity_magnitude * velocity_magnitude;
    // float drag_magnitude = density * drag_coefficient * velocity_magnitude * velocity_magnitude;
    // float drag_magnitude = (1-density) * drag_coefficient * velocity_magnitude * velocity_magnitude;
    drag_magnitude = min(velocity_magnitude, drag_magnitude); // Cap drag as to not push particles the other way
    new_velocity -= drag_magnitude * velocity_normal;

    out_velocity = vec4(new_velocity, 0.0, 1.0);
}
)";
