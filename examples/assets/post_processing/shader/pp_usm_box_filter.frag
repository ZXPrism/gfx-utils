#version 460 core

uniform sampler2D u_input_texture_sampler;
uniform vec2 u_window_size;

in vec2 m_uv;
out vec4 o_color;

void main() {
    vec2 texel_size = 1.0 / u_window_size;
    vec3 color = texture(u_input_texture_sampler, m_uv).xyz;
    if(dot(color, color) > 0.5) {
        o_color = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        o_color = vec4(color, 1.0);
    }
}
