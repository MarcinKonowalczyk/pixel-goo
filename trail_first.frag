#version 330 core
out vec4 color;

uniform sampler2D previous_trail_map;
uniform float alpha;

void main() {    
    vec2 previous_trail = texture(previous_trail_map, (gl_PointCoord.xy+1)*0.5).xy;
    // color = vec4( mix(0.0,1.0,(gl_PointCoord.x+1)/2), mix(1.0,0.0,(gl_PointCoord.y+1)/2), 0.0, 1.0);
    color = vec4(previous_trail, 0.0f, alpha);
}