#version 330 core

in vec2 world_texcoord;

uniform sampler2D image;

out vec4 FragColor;

void main() {
    FragColor = texture(image, world_texcoord);
}
