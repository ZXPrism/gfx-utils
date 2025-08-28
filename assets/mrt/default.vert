#version 460 core

layout(location = 0) in vec2 _Pos;
layout(location = 1) in vec2 _UV;

void main() {
    gl_Position = vec4(_Pos, 0.0, 1.0);
}
