#version 460 core

uniform sampler2D u_input_texture_sampler;
uniform vec2 u_window_size;

in vec2 m_uv;
out vec4 o_color;

uniform float strength;

const vec3 gray_weights = vec3(0.299, 0.587, 0.114);
const float gx[9] = float[9](-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0);
const float gy[9] = float[9](-1.0, -2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0);

void main() {
    vec2 texel_size = 1.0 / u_window_size;

    float gx_val = 0.0;
    float gy_val = 0.0;
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 uv = m_uv + vec2(x, y) * texel_size;
            gx_val += gx[(1 - y) * 3 + (x + 1)] * dot(gray_weights, texture(u_input_texture_sampler, uv).xyz);
            gy_val += gy[(1 - y) * 3 + (x + 1)] * dot(gray_weights, texture(u_input_texture_sampler, uv).xyz);
        }
    }
    float g = sqrt(gx_val * gx_val + gy_val * gy_val);

    o_color = vec4(clamp(strength * g + texture(u_input_texture_sampler, m_uv).xyz, 0.0, 1.0), 1.0);
}
