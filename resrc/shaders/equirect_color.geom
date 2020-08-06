#version 410 core

#define M_PI 3.1415926535897932384626433832795
#define EPSILON 0.000001
#define NEAR 0.01
#define FAR 500.0
#define LEFT -M_PI
#define RIGHT M_PI
#define BOTTOM (-M_PI / 2.0)
#define TOP (M_PI / 2.0)

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

in vec3 world_position_tese[];
in vec3 world_normal_tese[];
in vec2 model_texcoord_tese[];
in vec3 model_color_tese[];
in vec3 model_center_tese[];

const mat4 ortho_projection = mat4(
    vec4(2.0 / (RIGHT - LEFT), 0.0, 0.0, 0.0),
    vec4(0.0, 2.0 / (TOP - BOTTOM), 0.0, 0.0),
    vec4(0.0, 0.0, -2.0 / (FAR - NEAR), 0.0),
    vec4(-(RIGHT + LEFT) / (RIGHT - LEFT), -(RIGHT + LEFT) / (TOP - BOTTOM), -(FAR + NEAR) / (FAR - NEAR), 1.0)
);

uniform vec3 camera_position;
uniform float camera_offset;

out vec3 world_position;
out vec3 world_normal;
out vec2 model_texcoord;
out vec3 model_color;
out vec3 model_center;

float min3(vec3 v);
float max3(vec3 v);
void projectTriangle(vec3 verts[3], out vec4 projected_verts[3]);
vec4 equirectangular(vec3 vertex_position);
float projectedDistance(vec3 vertex_position);
vec2 lerp3D(vec2 v0, vec2 v1, vec2 v2, vec3 weights);
vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2, vec3 weights);
vec3 barycentric(vec3 v0, vec3 v1, vec3 v2, vec3 p);

void main() {
    //vec3 verts[3] = vec3[](gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);
    vec3 verts[3] = vec3[](world_position_tese[0], world_position_tese[1], world_position_tese[2]);

    /*
    // check if all three verts form a line - if so, do not render
    vec3 ab = verts[1] - verts[0];
    vec3 ac = verts[2] - verts[0];
    float triangle_area2 = length(cross(ab, ac));
    if (triangle_area2 < 2.0 * EPSILON) {
        return;
    }
    */

    // good triangle - perform equirectangular projection and find min/max longitude
    int i, j, num_verts;
    bool non_pole = true;
    vec3 origin = vec3(0.0, 0.0, 0.0);
    vec4 projected_verts[3];
    vec4 final_projected_verts[12];
    vec3 final_world_positions[12];
    vec3 final_world_normals[12];
    vec2 final_model_texcoords[12];
    projectTriangle(verts, projected_verts);

    vec3 lons = vec3(projected_verts[0].x, projected_verts[1].x, projected_verts[2].x);
    float min_lon = min3(lons);
    float max_lon = max3(lons);

    // offset camera - assume looking directly at each vertex
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 camera_position_v[3];
    for (i = 0; i < 3; i++) {
        vec3 dir = normalize(verts[i] - camera_position);
        vec3 right = cross(dir, up);
        vec3 offset = (length(right) > EPSILON) ? camera_offset * normalize(right) : vec3(0.0, 0.0, camera_offset);
        camera_position_v[i] = camera_position + offset;
    }

    // determine if triangle covers N or S pole
    vec3 v0_dir = vec3(verts[0].x - camera_position_v[0].x, 0.0, verts[0].z - camera_position_v[0].z);
    vec3 v1_dir = vec3(verts[1].x - camera_position_v[1].x, 0.0, verts[1].z - camera_position_v[1].z);
    vec3 v2_dir = vec3(verts[2].x - camera_position_v[2].x, 0.0, verts[2].z - camera_position_v[2].z);
    vec3 weights = barycentric(v0_dir, v1_dir, v2_dir, origin);

    // triangle covers N or S pole
    //if (weights.x >= 0.0 && weights.y >= 0.0 && weights.z >= 0.0) {
    if (weights.x >= -EPSILON && weights.y >= -EPSILON && weights.z >= -EPSILON) {
        // determine N vs S pole (only looking at 1 vertex)
        float projected_pole = sign(verts[0].y - camera_position_v[0].y);

        // pole crosses through a vertex
        if (length(v0_dir - origin) < EPSILON || length(v1_dir - origin) < EPSILON || length(v2_dir - origin) < EPSILON) {
            int pole_vert = (length(v0_dir - origin) < EPSILON) ? 0 : ((length(v1_dir - origin) < EPSILON) ? 1 : 2);
            num_verts = 4;
            int idx;
            for (i = 0; i < 2; i++) {
                idx = (pole_vert + i + 1) % 3;
                final_projected_verts[2 * i] = vec4(projected_verts[idx].x, projected_pole, projected_verts[pole_vert].zw);
                final_world_positions[2 * i] = verts[pole_vert];
                final_world_normals[2 * i] = world_normal_tese[pole_vert];
                final_model_texcoords[2 * i] = model_texcoord_tese[pole_vert];

                final_projected_verts[2 * i + 1] = projected_verts[idx];
                final_world_positions[2 * i + 1] = verts[idx];
                final_world_normals[2 * i + 1] = world_normal_tese[idx];
                final_model_texcoords[2 * i + 1] = model_texcoord_tese[idx];
            }
        }
        // pole crosses through an edge
        else if (weights.x < EPSILON || weights.y < EPSILON || weights.z < EPSILON) {
            vec3 pole_intersect_position = lerp3D(verts[0], verts[1], verts[2], weights);
            vec3 pole_intersect_normal = normalize(lerp3D(world_normal_tese[0], world_normal_tese[1], world_normal_tese[2], weights));
            vec2 pole_intersect_texcoord = lerp3D(model_texcoord_tese[0], model_texcoord_tese[1], model_texcoord_tese[2], weights);
            float pole_intersect_distance = projectedDistance(pole_intersect_position);

            int indices[3] = (weights.x < EPSILON) ? int[](2, 0, 1) : ((weights.y < EPSILON) ? int[](0, 1, 2) : int[](1, 2, 0));
            num_verts = 6;
            for (i = 0; i < 3; i++) {
                final_projected_verts[2 * i] = vec4(projected_verts[indices[i]].x, projected_pole, pole_intersect_distance, 1.0);
                final_world_positions[2 * i] = pole_intersect_position;
                final_world_normals[2 * i] = pole_intersect_normal;
                final_model_texcoords[2 * i] = pole_intersect_texcoord;

                final_projected_verts[2 * i + 1] = projected_verts[indices[i]];
                final_world_positions[2 * i + 1] = verts[indices[i]];
                final_world_normals[2 * i + 1] = world_normal_tese[indices[i]];
                final_model_texcoords[2 * i + 1] = model_texcoord_tese[indices[i]];
            }
        }
        // pole crosses through center of triangle
        else {
            non_pole = false;
            vec3 pole_intersect_position = lerp3D(verts[0], verts[1], verts[2], weights);
            vec3 pole_intersect_normal = normalize(lerp3D(world_normal_tese[0], world_normal_tese[1], world_normal_tese[2], weights));
            vec2 pole_intersect_texcoord = lerp3D(model_texcoord_tese[0], model_texcoord_tese[1], model_texcoord_tese[2], weights);
            float pole_intersect_distance = projectedDistance(pole_intersect_position);

            int indices[3];
            for (i = 0; i < 3; i++) {
                int idx = i;
                float x0 = projected_verts[i].x;
                x0 = (x0 > 0.0) ? x0 - 2.0 : x0;
                for (j = i - 1; j >= 0; j--) {
                    float x1 = projected_verts[indices[j]].x;
                    x1 = (x1 > 0.0) ? x1 - 2.0 : x1;
                    if (x1 > x0) {
                        indices[j+1] = indices[j];
                        idx = j;
                    }
                }
                indices[idx] = i;
            }
            num_verts = 6;
            for (i = 0; i < 3; i++) {
                final_projected_verts[2 * i] = vec4(projected_verts[indices[i]].x, projected_pole, pole_intersect_distance, 1.0);
                final_world_positions[2 * i] = pole_intersect_position;
                final_world_normals[2 * i] = pole_intersect_normal;
                final_model_texcoords[2 * i] = pole_intersect_texcoord;

                final_projected_verts[2 * i + 1] = projected_verts[indices[i]];
                final_world_positions[2 * i + 1] = verts[indices[i]];
                final_world_normals[2 * i + 1] = world_normal_tese[indices[i]];
                final_model_texcoords[2 * i + 1] = model_texcoord_tese[indices[i]];
            }
        }
    }
    else {
        num_verts = 3;
        for (i = 0; i < 3; i++) {
            final_projected_verts[i] = projected_verts[i];
            final_world_positions[i] = verts[i];
            final_world_normals[i] = world_normal_tese[i];
            final_model_texcoords[i] = model_texcoord_tese[i];
        }
    }

    
    // emit triangle strips
    if (max_lon - min_lon > 1.0) { // triangle crosses the x=0 plane while z <= 0 (i.e. wraps around left-right edges)
        // left side
        for (i = 0; i < num_verts; i++) {
            final_projected_verts[i].x = (final_projected_verts[i].x > 0.0) ? final_projected_verts[i].x - 2.0 : final_projected_verts[i].x;

            world_position = final_world_positions[i];
            world_normal = final_world_normals[i];
            model_texcoord = final_model_texcoords[i];
            model_color = model_color_tese[0]; // all vertices have same model color
            model_center = model_center_tese[0]; // all vertices have same model center
            gl_Position = final_projected_verts[i];
            EmitVertex();
        }
        if (non_pole) EndPrimitive();

        // right side
        for (i = 0; i < num_verts; i++) {
            final_projected_verts[i].x += 2.0;

            world_position = final_world_positions[i];
            world_normal = final_world_normals[i];
            model_texcoord = final_model_texcoords[i];
            model_color = model_color_tese[0]; // all vertices have same model color
            model_center = model_center_tese[0]; // all vertices have same model center
            gl_Position = final_projected_verts[i];
            EmitVertex();
        }
        EndPrimitive();
    }
    else {
        for (i = 0; i < num_verts; i++) {
            world_position = final_world_positions[i];
            world_normal = final_world_normals[i];
            model_texcoord = final_model_texcoords[i];
            model_color = model_color_tese[0]; // all vertices have same model color
            model_center = model_center_tese[0]; // all vertices have same model center
            gl_Position = final_projected_verts[i];
            EmitVertex();
        }
        EndPrimitive();
    }
    
}

float min3(vec3 v) {
  return min(min(v.x, v.y), v.z);
}

float max3(vec3 v) {
  return max(max(v.x, v.y), v.z);
}

void projectTriangle(vec3 verts[3], out vec4 projected_verts[3]) {
    int i;
    for (i = 0; i < 3; i++) {
        projected_verts[i] = equirectangular(verts[i]);
    }
}

vec4 equirectangular(vec3 vertex_position) {
    /*
    // move camera inside original projection sphere
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 dir = normalize(vertex_position - camera_position);
    vec3 offset = camera_offset * normalize(cross(dir, up));
    vec3 cam = camera_position + offset;

    
    // RAY-SPHERE INTERSECTION METHOD
    vec3 p1 = cam;
    vec3 p2 = vertex_position;
    vec3 p3 = camera_position;
    float radius = (camera_offset == 0.0) ? 1.0 : 50.0 * abs(camera_offset);

    vec3 d = p2 - p1;
    float magnitude = length(d);

    float a = dot(d, d);
    float b = 2.0 * dot(d, p1 - p3);
    float c = dot(p3, p3) + dot(p1, p1) - 2.0 * dot(p3, p1) - radius * radius;

    float test = (b * b) - (4.0 * a * c);
    float u = (-b + sqrt(test)) / (2.0 * a);
    vec3 hitp = p1 + u * (p2 - p1);

    // use hitp with original camera
    vec3 vertex_direction_p = hitp - camera_position;
    float magnitude_p = length(vertex_direction_p);
    float longitude = -atan(vertex_direction_p.x, vertex_direction_p.z);
    float latitude = asin(vertex_direction_p.y / magnitude_p);

    vec4 projected_vertex_position = ortho_projection * vec4(longitude, latitude, -magnitude, 1.0);
    return projected_vertex_position;
    */
    // move projection sphere with camera offset
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 dir = vertex_position - camera_position;
    vec3 right = cross(dir, up);

    // reduce ocular offset linearly starting at M_PI / 6.0 radians (30 degrees) away from a pole
    // TODO: try falloff between 30 - 10, 0 stereo from 10 to pole. Maybe quadratic? 
    float inclination = abs(asin(dir.y / length(dir))) / M_PI;
    //float adjusted_offset = clamp(1.0 - (6.0 * (inclination - 0.333333)), 0.0, 1.0) * camera_offset;
    // 1/6 = 0.1666667 1/18 = 0.055556, range = [0.333333, 0.444444]
    float adjust_start = 0.5 - (1.0 / 6.0); // 30 degrees from pole
    float adjust_end = 0.5 - (1.0 / 18.0); // 10 degrees from pole
    float adjusted_offset = clamp(1.0 - ((inclination - adjust_start) / (adjust_end - adjust_start)), 0.0, 1.0);

    //vec3 offset = (length(right) > EPSILON) ? camera_offset * normalize(right) : vec3(0.0, 0.0, camera_offset);
    vec3 offset = (length(right) > EPSILON) ? adjusted_offset * normalize(right) : vec3(0.0, 0.0, 0.0);
    vec3 cam = camera_position + offset;

    vec3 vertex_direction = vertex_position - cam;
    float magnitude = length(vertex_direction);
    float longitude = (abs(vertex_direction.z) < EPSILON) ? sign(vertex_direction.x) * -M_PI * 0.5 : -atan(vertex_direction.x, vertex_direction.z);
    //float longitude = -atan(vertex_direction.x, vertex_direction.z);
    float latitude = asin(vertex_direction.y / magnitude);

    vec4 projected_vertex_position = ortho_projection * vec4(longitude, latitude, -magnitude, 1.0);
    return projected_vertex_position;
}


float projectedDistance(vec3 vertex_position) {
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 dir = normalize(vertex_position - camera_position);
    vec3 right = cross(dir, up);
    vec3 offset = (length(right) > EPSILON) ? camera_offset * normalize(right) : vec3(0.0, 0.0, camera_offset);
    vec3 cam = camera_position + offset;

    vec3 vertex_direction = vertex_position - cam;
    float magnitude = length(vertex_direction);

    return (2.0 * magnitude / (FAR - NEAR)) - ((FAR + NEAR) / (FAR - NEAR));
}

vec2 lerp3D(vec2 v0, vec2 v1, vec2 v2, vec3 weights) {
    return weights.x * v0 + weights.y * v1 + weights.z * v2;
}

vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2, vec3 weights) {
    return weights.x * v0 + weights.y * v1 + weights.z * v2;
}

vec3 barycentric(vec3 v0, vec3 v1, vec3 v2, vec3 p) {
    /*
    vec3 ab = v1 - v0;
    vec3 ac = v2 - v0;

    float triangle_area2 = length(cross(ab, ac));
    if (triangle_area2 < 2.0 * EPSILON) {
        return vec3(-1.0, -1.0, -1.0);
    }

    vec3 ap = p - v0;

    float dbb = dot(ab, ab);
    float dbc = dot(ab, ac);
    float dcc = dot(ac, ac);
    float dbp = dot(ab, ap);
    float dcp = dot(ac, ap);

    float inv_denom = 1.0 / ((dbb * dcc) - (dbc * dbc));

    float w1 = ((dcc * dbp) - (dbc * dcp)) * inv_denom;
    float w2 = ((dbb * dcp) - (dbc * dbp)) * inv_denom;
    float w0 = 1.0 - w1 - w2;

    return vec3(w0, w1, w2);
*/

    vec3 edge01 = v1 - v0;
    vec3 edge02 = v2 - v0;
    vec3 n = cross(edge01, edge02);
    if (length(n) < 2.0 * EPSILON) {
        return vec3(-1.0, -1.0, -1.0);
    }
    float inv_denom = 1.0 / dot(n, n);

    // edge 0 (opposite of v0)
    vec3 edge0 = v2 - v1;
    vec3 vp1 = p - v1;
    vec3 c0 = cross(edge0, vp1);
    float u = dot(n, c0) * inv_denom;

    // edge 1 (opposite of v1)
    vec3 edge1 = v0 - v2;
    vec3 vp2 = p - v2;
    vec3 c1 = cross(edge1, vp2);
    float v = dot(n, c1) * inv_denom;

    // edge 2 (opposite of v2)
    float w = 1.0 - u - v;

    return vec3(u, v, w);
}
