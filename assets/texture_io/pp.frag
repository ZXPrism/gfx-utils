#version 460 core

uniform sampler2D input_texture_sampler;
uniform int window_width;
uniform int window_height;

in vec2 uv;
out vec4 _Color;

void main() {
    vec2 texel_size = vec2(1.0 / float(window_width), 1.0 / float(window_height));

    // simple box filter
    vec3 res = vec3(0.0);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            vec2 sample_uv = uv + texel_size * vec2(float(x), float(y));
            res += texture(input_texture_sampler, sample_uv).xyz;
        }
    }
    res /= 9.0;

    _Color = vec4(res, 1.0);
}
