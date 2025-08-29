#version 460 core

layout(std430, binding = 0) buffer InputBuffer {
	vec4 data[];
};

out vec4 _Color;

uniform int window_width;

void main() {
    uint idx = uint(floor(gl_FragCoord.y)) * window_width + uint(floor(gl_FragCoord.x));
    _Color = data[idx];
}
