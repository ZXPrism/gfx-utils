#version 460 core

out vec4 _Color;

uniform int option;
uniform sampler2D color_r_sampler;
uniform sampler2D color_g_sampler;
uniform sampler2D color_b_sampler;

void main() {
    if(option == 0) {
        _Color = texture(color_r_sampler, vec2(0.0));
    } else if(option == 1) {
        _Color = texture(color_g_sampler, vec2(0.0));
    } else {
        _Color = texture(color_b_sampler, vec2(0.0));
    }
}
