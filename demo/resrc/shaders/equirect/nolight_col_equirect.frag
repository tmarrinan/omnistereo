#version 410 core

in vec3 world_position;
in vec3 world_normal;
in vec2 model_texcoord;

uniform vec3 material_color;

out vec4 FragColor;

void main() {
    FragColor = vec4(material_color, 1.0);
}
