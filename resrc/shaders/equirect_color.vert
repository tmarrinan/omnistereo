#version 410 core

#define EPSILON 0.000001

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texcoord;
in vec3 point_center;
in vec3 point_color;
in float point_size;

//uniform vec3 model_center;
//uniform float model_size;
uniform vec3 camera_position;
uniform float camera_offset;

out vec3 world_position_vert;
out vec3 world_normal_vert;
out vec2 model_texcoord_vert;
out vec3 model_color_vert;
out vec3 model_center_vert;

void main() {
    vec3 vertex_direction = normalize(point_center - camera_position);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(vertex_direction, up);
    vec3 offset = (length(right) > EPSILON) ? camera_offset * normalize(right) : vec3(0.0, 0.0, camera_offset);
    vec3 cam = camera_position + offset;

    vertex_direction = normalize(point_center - cam);
    right = cross(vertex_direction, up);
    vec3 cam_right = (length(right) > EPSILON) ? normalize(right) : vec3(0.0, 0.0, 1.0);
    vec3 cam_up = cross(cam_right, vertex_direction);

    world_position_vert = point_center + cam_right * vertex_position.x * point_size +
                                         cam_up * vertex_position.y * point_size;
    world_normal_vert = -vertex_direction;
    model_texcoord_vert = vertex_texcoord;
    model_color_vert = point_color;
    model_center_vert = point_center;


    //world_position_vert = (model_size * vertex_position) + point_center;
    //world_normal_vert = vertex_normal;
    //model_texcoord_vert = vertex_texcoord;
    //model_color_vert = point_color;
    //model_center_vert = point_center;
}
