#version 460 core

uniform sampler2D u_input_texture_sampler;
uniform vec2 u_window_size;

in vec2 m_uv;
out vec4 o_color;

void main() {
    o_color = texture(u_input_texture_sampler, m_uv);
}
