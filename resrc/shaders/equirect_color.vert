#version 410 core

in vec3 vertex_position;
in vec3 vertex_normal;
in vec3 point_center;
in vec3 point_color;

uniform vec3 model_center;
uniform float model_size;

out vec3 world_position_vert;
out vec3 world_normal_vert;
out vec3 model_color_vert;

void main() {
    //world_position_vert = vertex_position + point_center;
    //world_normal_vert = vertex_normal;
    model_color_vert = point_color;

    world_position_vert = (model_size * vertex_position) + model_center;
    world_normal_vert = vertex_normal;
}
