#version 410 core

in vec2 world_position;
in vec2 world_normal;
in vec2 model_texcoord;

uniform sampler2D image;

out vec4 FragColor;

void main() {
    FragColor = texture(image, model_texcoord);
}
