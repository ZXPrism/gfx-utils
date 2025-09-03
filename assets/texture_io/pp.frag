#version 460 core

uniform sampler2D input_texture_sampler;
uniform int window_width;
uniform int window_height;

in vec2 uv;
out vec4 _Color;

const float kernel[9] = float[9](0.0, -0.2, 0.0, -0.2, 0.8, -0.2, 0.0, -0.2, 0.0);
const vec3 gray_weights = vec3(0.299, 0.587, 0.114);

void main() {
    vec2 texel_size = vec2(1.0 / float(window_width), 1.0 / float(window_height));

    // simple sharpening
    float res = 0.0;
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 sample_uv = uv + texel_size * vec2(float(x), float(y));
            res += kernel[(1 - y) * 3 + x + 1] * dot(gray_weights, texture(input_texture_sampler, sample_uv).xyz);
        }
    }

    _Color = vec4(5.0 * vec3(res), 1.0);
}
