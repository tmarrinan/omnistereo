#version 410 core

in vec3 vertex_position;
in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

out vec3 world_position_vert;
out vec3 world_normal_vert;

void main() {
    world_position_vert = (model_matrix * vec4(vertex_position, 1.0)).xyz;
    world_normal_vert = normalize(normal_matrix * vertex_normal);
}
