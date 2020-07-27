#version 410 core

in vec3 vertex_position;
in vec2 vertex_texcoord;

uniform mat4 model_matrix;

out vec3 world_position_vert;
out vec3 world_normal_vert;
out vec2 model_texcoord_vert;

void main() {
    vec4 position = model_matrix * vec4(vertex_position, 1.0);

    world_position_vert = position.xyz;
    world_normal_vert = vec3(0.0, 0.0, 0.0); // unused for nolight_tex shader
    model_texcoord_vert = vertex_texcoord;
}
