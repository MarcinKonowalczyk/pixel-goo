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
uniform vec2 window_size;
uniform vec2 mouse_position;

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

// Helper funciton to sample textures
// TExtures are smaples from the *bottom* left corner, and in normalised coordinates
vec2 toNormalisedCoords(vec2 coordinate) {
    coordinate = mod(coordinate, window_size);
    return vec2(
        +coordinate.x/window_size.x,
        -(coordinate.y/window_size.y)
        );
}

#define PI 3.141592653589793
#define GOLD 1.618033988749895

void main() {
    int i = int(gl_FragCoord.x); // Index of the particle
    vec2 position = vec2(texelFetch(position_buffer, i, 0)); // current position
    vec2 velocity = vec2(texelFetch(velocity_buffer, i, 0)); // previous velocity
    float density = texture(density_map, toNormalisedCoords(position)).x;

    vec2 new_velocity = velocity;

    // Integrate density over a disk in a radius
    vec2 density_integral = vec2(0,0);
    int N = 300;
    for (int i = 0; i < N; i ++) {
        // float r = sqrt((i+0.5)/N);
        float r = (i+0.5)/N * (i+0.5)/N;
        // float r = (i+0.5)/N;
        float theta = 2*PI*GOLD * (i+0.5);
        float phase = PI*random(vec2(0,1) + VertexID + epoch_counter).y;
        // float phase = 0;
        vec2 sample_xy = r * vec2( cos(theta+phase), sin(theta+phase) ) * 100;
        // vec2 sample_xy = vec2(+position.x,-position.y) + sample_delta_xy;

        float density_sample = texture(density_map, toNormalisedCoords(position + sample_xy)).x;
        density_integral += density_sample * sample_xy;
    }
    // new_velocity -= 0.09*density_integral;
    new_velocity -= 0.001*density_integral;

    // Mouse repell
    const float mouse_repell_radius = 1000;
    vec2 mouse_vector = mouse_position-position;
    float mouse_vector_length = length(mouse_vector);
    if (mouse_vector_length < 400) {
        vec2 mouse_vector_normal;
        if (mouse_vector_length > 0) {
            mouse_vector_normal = mouse_vector / mouse_vector_length;
        } else {
            mouse_vector_normal = random(vec2(0,2) + VertexID + epoch_counter);
            mouse_vector_normal /= length(mouse_vector_normal);
        }
        new_velocity -= (1-(mouse_vector_length/400)) * (1-(mouse_vector_length/400)) * mouse_vector_normal;
    }

    // Dither
    // new_velocity += dither_coefficient * random(vec2(0,0) + VertexID + epoch_counter);
    // new_velocity += clamp(1-density,0.2,1.0) * dither_coefficient * random(vec2(0,0) + VertexID + epoch_counter);
    new_velocity += density * dither_coefficient * random(vec2(0,0) + VertexID + epoch_counter);


    // Drift
    // new_velocity += 0.1 * vec2(1.0, 1.0);
    // new_velocity += (1-density) * vec2(1.0, 1.0);
    // new_velocity += density * 0.1 * vec2(3.0, 2.0);

    // Resolve drag after all other acceleration to make sure that very high drag coefficient works
    float old_velocity_magnitude = length(velocity);
    float velocity_magnitude = length(new_velocity);
    vec2 velocity_normal;
    if (velocity_magnitude == 0) { // Return random nomral if magnitude is zero
        velocity_normal = random(vec2(1,0) + VertexID + epoch_counter);
        velocity_normal /= length(velocity_normal);
    } else {
        velocity_normal = new_velocity / velocity_magnitude;
    }

    // float drag_magnitude = drag_coefficient * velocity_magnitude * velocity_magnitude;
    float drag_magnitude = drag_coefficient * old_velocity_magnitude * old_velocity_magnitude;

    // float drag_magnitude = density * drag_coefficient * velocity_magnitude * velocity_magnitude;
    // float drag_magnitude = (1-density) * drag_coefficient * velocity_magnitude * velocity_magnitude;
    drag_magnitude = min(velocity_magnitude, drag_magnitude); // Cap drag as to not push particles the other way
    new_velocity -= drag_magnitude * velocity_normal;

    if (any(isnan(new_velocity)) || any(isinf(new_velocity))) {
        new_velocity = vec2(0,0);
    }

    out_velocity = vec4(new_velocity, 0.0, 1.0);
}
)";
