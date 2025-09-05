#version 460 core

layout(location = 0) in vec2 i_pos;
layout(location = 1) in vec2 i_uv;

out vec2 m_uv;

void main() {
    m_uv = i_uv;
    gl_Position = vec4(i_pos, 0.0, 1.0);
}
