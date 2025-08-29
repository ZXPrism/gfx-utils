#version 460 core

out vec4 _Color;

// layout(std430, binding = 0) buffer DataBuffer {
// float data[];
// };

void main() {
    _Color = vec4(1.0, 0.0, 0.0, 1.0);
}
