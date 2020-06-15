#version 410 core

#define M_PI 3.1415926535897932384626433832795
#define NEAR 0.01
#define FAR 1000.0

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

in vec3 world_normal_tese[];

uniform vec3 camera_position;

out vec3 world_position;
out vec3 world_normal;

float min3 (vec3 v);
float max3 (vec3 v);
vec4 equirectangular(vec3 vertex_position);
vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2, vec3 weights);
vec3 barycentric(vec3 v0, vec3 v1, vec3 v2, vec3 p);

void main() {
    int i, j;

    vec3 verts[3];
    verts[0] = gl_in[0].gl_Position.xyz;
    verts[1] = gl_in[1].gl_Position.xyz;
    verts[2] = gl_in[2].gl_Position.xyz;

    vec3 x_coords = vec3(verts[0].x, verts[1].x, verts[2].x);
    vec3 z_coords = vec3(verts[0].z, verts[1].z, verts[2].z);
    float min_x = min3(x_coords);
    float max_x = max3(x_coords);
    float min_z = min3(z_coords);

    vec3 ab = verts[1] - verts[0];
    vec3 ac = verts[2] - verts[0];
    vec3 plane = normalize(cross(ab, ac));
    float neg_d = dot(plane, verts[0]);
    vec3 y_intersect = vec3(0.0, neg_d / plane.y, 0.0); //TODO: take camera position into account
    vec3 weights = barycentric(verts[0], verts[1], verts[2], y_intersect);

    // all three verts form a line - skip
    if (abs(dot(normalize(ab), normalize(ac))) >= 0.9999) {
        return;
    }

    if (weights.x >= 0.0 && weights.y >= 0.0 && weights.z >= 0.0 && plane.y != 0.0) { // N or S pole in triangle
        
        vec3 verts_sortx[3];
        vec4 projected_position_array[3];
        float projected_pole = sign(verts[0].y - camera_position.y); // determine N vs S pole (only looking at 1 vertex)
        
        vec3 y_intersect_position = lerp3D(verts[0], verts[1], verts[2], weights);
        vec3 y_intersect_normal = normalize(lerp3D(world_normal_tese[0], world_normal_tese[1], world_normal_tese[2], weights));
        
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            vec3 v_world_position = verts[i];
            vec4 v_projected_position = equirectangular(v_world_position);
            if (v_projected_position.x > 0.0) {
                v_projected_position.x -= 2.0;
            }
            int position = i;
            for (j = i - 1; j >= 0; j--) {
                if (projected_position_array[j].x > v_projected_position.x) {
                    verts_sortx[j+1] = verts_sortx[j];
                    projected_position_array[j + 1] = projected_position_array[j];
                    position = j;
                }
            }
            verts_sortx[position] = v_world_position;
            projected_position_array[position] = v_projected_position;
        }

        // left side
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            world_position = y_intersect_position;
            world_normal = y_intersect_normal;
            gl_Position = vec4(projected_position_array[i].x, projected_pole, projected_position_array[i].zw); // TODO: recalculate z
            EmitVertex();

            world_position = verts_sortx[i];
            world_normal = world_normal_tese[i];
            gl_Position = projected_position_array[i];
            EmitVertex();
        }

        // right side
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            projected_position_array[i].x += 2.0;

            world_position = y_intersect_position;
            world_normal = y_intersect_normal;
            gl_Position = vec4(projected_position_array[i].x, projected_pole, projected_position_array[i].zw); // TODO: recalculate z
            EmitVertex();

            world_position = verts_sortx[i];
            world_normal = world_normal_tese[i];
            gl_Position = projected_position_array[i];
            EmitVertex();
        }

        EndPrimitive();
        
    }
    else if (min_x * max_x <= 0.0 && min_z <= 0.0) { // triangle crosses the x=0 plane while z <= 0 (i.e. wraps around left-right edges)
        // draw triangle twice - once on left of screen, once on right
        //vec3 world_position_array[3];
        vec4 projected_position_array[3];

        // left side
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            projected_position_array[i] = equirectangular(verts[i]);
            if (projected_position_array[i].x > 0.0) {
                projected_position_array[i].x -= 2.0;
            }

            world_position = verts[i];
            world_normal = world_normal_tese[i];
            gl_Position = projected_position_array[i];
            EmitVertex();
        }
        EndPrimitive();

        // right side
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            projected_position_array[i].x += 2.0;

            world_position = verts[i];
            world_normal = world_normal_tese[i];
            gl_Position = projected_position_array[i];
            EmitVertex();
        }
        EndPrimitive();
    }
    else { // not a special case - draw triangle normally
        for (i = 0; i < 3; i++) { // triangles, so it's always 3
            world_position = verts[i];
            world_normal = world_normal_tese[i];
            gl_Position = equirectangular(world_position);
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

vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2, vec3 weights) {
    return weights.x * v0 + weights.y * v1 + weights.z * v2;
}

vec3 barycentric(vec3 v0, vec3 v1, vec3 v2, vec3 p) {
    vec3 ab = v1 - v0;
    vec3 ac = v2 - v0;
    vec3 ap = p - v0;

    float d00 = dot(ab, ab);
    float d01 = dot(ab, ac);
    float d11 = dot(ac, ac);
    float d20 = dot(ap, ab);
    float d21 = dot(ap, ac);
    float inv_denom = 1.0 / ((d00 * d11) - (d01 * d01));

    float w1 = ((d11 * d20) - (d01 * d21)) * inv_denom;
    float w2 = ((d00 * d21) - (d01 * d20)) * inv_denom;
    float w0 = 1.0 - w1 - w2;

    return vec3(w0, w1, w2);
}
