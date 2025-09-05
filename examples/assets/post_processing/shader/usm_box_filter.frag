#version 460 core

uniform sampler2D u_input_texture_sampler;
uniform vec2 u_window_size;

in vec2 m_uv;
out vec4 o_color;

uniform float strength;

const vec3 gray_weights = vec3(0.299, 0.587, 0.114);
const float[9] kernel = float[9](//
-1.0 / 9.0, -1.0 / 9.0, -1.0 / 9.0, //
-1.0 / 9.0, 8.0 / 9.0, -1.0 / 9.0, //
-1.0 / 9.0, -1.0 / 9.0, -1.0 / 9.0); //

void main() {
    vec2 texel_size = 1.0 / u_window_size;

    float edge = 0.0;
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 uv = m_uv + vec2(x, y) * texel_size;
            edge += kernel[(1 - y) * 3 + (x + 1)] * dot(texture(u_input_texture_sampler, uv).xyz, gray_weights);
        }
    }

    o_color = vec4(clamp(strength * edge + texture(u_input_texture_sampler, m_uv).xyz, 0.0, 1.0), 1.0);
}
