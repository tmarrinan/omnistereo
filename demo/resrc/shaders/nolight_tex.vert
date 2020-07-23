#version 330 core

in vec3 vertex_position;
in vec2 vertex_texcoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec2 world_texcoord;

void main() {
    world_texcoord = vertex_texcoord;

    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(vertex_position, 1.0);
}
