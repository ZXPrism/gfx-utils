#version 460 core

uniform sampler2D input_texture_sampler;

in vec2 uv;
out vec4 _Color;

void main() {
    _Color = texture(input_texture_sampler, uv);
}
