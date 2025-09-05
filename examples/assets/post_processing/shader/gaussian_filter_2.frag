#version 460 core

uniform sampler2D u_input_texture_sampler;
uniform vec2 u_window_size;

in vec2 m_uv;
out vec4 o_color;

void main() {
    vec2 texel_size = 1.0 / u_window_size;

    vec3 res = vec3(0.0);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 sample_uv = m_uv + texel_size * vec2(float(x), float(y));
            res += texture(u_input_texture_sampler, sample_uv).xyz;
        }
    }
    res /= 9.0;

    o_color = vec4(res, 1.0);
}
