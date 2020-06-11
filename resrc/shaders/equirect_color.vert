#version 330 core

precision highp float;

in vec3 vertex_position;
in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

out vec3 world_normal_geom;

void main() {
    world_normal_geom = normalize(normal_matrix * vertex_normal);

    gl_Position = model_matrix * vec4(vertex_position, 1.0);
}
