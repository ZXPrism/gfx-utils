#version 460 core

in vec2 uv;
out vec4 _Color;

uniform sampler2D albedo_sampler;

void main() {
    _Color = texture(albedo_sampler, uv);
}
