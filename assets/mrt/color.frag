#version 460 core

layout(location = 0) out vec4 _Color_r;
layout(location = 1) out vec4 _Color_g;
layout(location = 2) out vec4 _Color_b;

void main() {
    _Color_r = vec4(1.0, 0.0, 0.0, 1.0);
    _Color_g = vec4(0.0, 1.0, 0.0, 1.0);
    _Color_b = vec4(0.0, 0.0, 10.0, 1.0);
}
