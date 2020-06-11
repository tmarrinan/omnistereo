#version 330 core

precision highp float;

#define M_PI 3.1415926535897932384626433832795
#define NEAR 0.01
#define FAR 1000.0

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

in vec3 world_normal_geom[];

uniform vec3 camera_position;

out vec3 world_position;
out vec3 world_normal;

float min3 (vec3 v);
float max3 (vec3 v);
vec4 equirectangular(vec3 vertex_position);

void main() {
    int i;
    vec3 x_coords = vec3(gl_in[0].gl_Position.x, gl_in[1].gl_Position.x, gl_in[2].gl_Position.x);
    vec3 z_coords = vec3(gl_in[0].gl_Position.z, gl_in[1].gl_Position.z, gl_in[2].gl_Position.z);
    float min_x = min3(x_coords);
    float max_x = max3(x_coords);
    float min_z = min3(z_coords);

    if (min_x * max_x > 0.0 || min_z > 0.0) { // triangle does NOT cross the x=0 plane while z <= 0
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            world_position = vec3(gl_in[i].gl_Position);
            world_normal = world_normal_geom[i];
            gl_Position = equirectangular(world_position);
            EmitVertex();
        }
        EndPrimitive();
    }
    else {
        // draw triangle twice - once on left of screen, once on right
        vec3 world_position_array[3];
        vec4 projected_position_array[3];

        // left side
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            world_position_array[i] = vec3(gl_in[i].gl_Position);
            projected_position_array[i] = equirectangular(world_position_array[i]);
            if (projected_position_array[i].x > 0.0) {
                projected_position_array[i].x -= 2.0;
            }

            world_position = world_position_array[i];
            world_normal = world_normal_geom[i];
            gl_Position = projected_position_array[i];
            EmitVertex();
        }
        EndPrimitive();

        // right side
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            projected_position_array[i].x += 2.0;

            world_position = world_position_array[i];
            world_normal = world_normal_geom[i];
            gl_Position = projected_position_array[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}

float min3 (vec3 v) {
  return min(min(v.x, v.y), v.z);
}

float max3 (vec3 v) {
  return max(max(v.x, v.y), v.z);
}

vec4 equirectangular(vec3 vertex_position) {
    vec3 vertex_direction = vertex_position - camera_position;
    float magnitude = length(vertex_direction);
    float longitude = atan(vertex_direction.x, vertex_direction.z);
    float latitude = atan(vertex_direction.y, length(vertex_direction.zx)); //asin(vertex_direction.y / magnitude);

    return vec4(-longitude / M_PI, 2.0 * latitude / M_PI, (magnitude - NEAR) / (FAR - NEAR), 1.0);
}
