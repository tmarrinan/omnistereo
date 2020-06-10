#version 330 core

precision highp float;

in vec3 vertex_position;
in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

out vec3 world_position;
out vec3 world_normal;

void main() {
    world_position = vec3(model_matrix * vec4(vertex_position, 1.0));
    world_normal = normalize(normal_matrix * vertex_normal);

    // change for equirectangular projection
    gl_Position = vec4(world_position, 1.0);
}
