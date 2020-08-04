#version 410 core

#define M_PI 3.1415926535897932384626433832795
#define EPSILON 0.000001

layout(vertices = 3) out;

in vec3 world_position_vert[];
in vec3 world_normal_vert[];
in vec2 model_texcoord_vert[];
in vec3 model_color_vert[];
in vec3 model_center_vert[];

uniform vec3 camera_position;

out vec3 world_position_tesc[];
out vec3 world_normal_tesc[];
out vec2 model_texcoord_tesc[];
out vec3 model_color_tesc[];
out vec3 model_center_tesc[];

const float toDegrees = 180.0 / M_PI;

float min3(float v[3]);
float min3(float v1, float v2, float v3);
float max3(float v[3]);
float max3(float v1, float v2, float v3);

void main() {
    world_position_tesc[gl_InvocationID] = world_position_vert[gl_InvocationID];
    world_normal_tesc[gl_InvocationID] = world_normal_vert[gl_InvocationID];
    model_texcoord_tesc[gl_InvocationID] = model_texcoord_vert[gl_InvocationID];
    model_color_tesc[gl_InvocationID] = model_color_vert[gl_InvocationID];
    model_center_tesc[gl_InvocationID] = model_center_vert[gl_InvocationID];

    if (gl_InvocationID == 0) {
        // TODO: take into account camera_offset
        int i;
        vec3 vertex_direction;
        float longitude[3];
        float latitude[3];
        float midpoint_latitude[3];
        for (i = 0; i < 3; i++){
            vertex_direction = world_position_vert[i] - camera_position;
            longitude[i] = (abs(vertex_direction.z) < EPSILON) ? sign(vertex_direction.x) * -90.0 : -atan(vertex_direction.x, vertex_direction.z) * toDegrees;
            //longitude[i] = -atan(vertex_direction.x, vertex_direction.z) * toDegrees;
            latitude[i] = asin(vertex_direction.y / length(vertex_direction)) * toDegrees;

            vertex_direction = (0.5 * (world_position_vert[(i + 1) % 3] + world_position_vert[(i + 2) % 3])) - camera_position;
            midpoint_latitude[i] = asin(vertex_direction.y / length(vertex_direction)) * toDegrees;
        }

        // subdivisions increase as edge covers a larger longitudinal span
        // subdivisions increase as the edge moves further awar from the equator
        float delta_lon;
        float delta_lat;
        float max_lat;
        float dist;
        float scalar;

        // 4.0 = cube root of 64
        // 3.1748 = cube root of 32 -- check with Mark
        // 2.51984 = cube root of 16
        // 2.8284 = square root of 8

        // subdivisions along v1,v2 edge
        //delta_lon = abs(longitude[1] - longitude[2]);
        delta_lon = min(min(abs(longitude[1] - longitude[2]), abs((longitude[1] - 360.0) - longitude[2])), abs(longitude[1] - (longitude[2] - 360.0)));
        delta_lat = max(max(abs(latitude[1] - latitude[2]), abs(latitude[1] - midpoint_latitude[0])), abs(latitude[2] - midpoint_latitude[0]));
        max_lat = max(max(abs(latitude[1]), abs(latitude[2])), abs(midpoint_latitude[0]));
        //float subdivisions_12 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 1.0) * max((32.0 / 180.0) * delta_lon, 1.0);
        //float subdivisions_12 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 0.75) * max((16.0 / 90.0) * delta_lat, 1.0) * max((32.0 / 180.0) * delta_lon, 0.75);
        dist = max(0.125 * length(vec2(delta_lon, 0.5 * delta_lat)), 1.0);
        scalar = max((2.8284 / 90.0) * max_lat, 1.0);
        scalar *= 1.0;//scalar;
        float subdivisions_12 = clamp(ceil(scalar * dist), 1.0, 64.0);

        // subdivisions along v2,v0 edge
        //delta_lon = abs(longitude[2] - longitude[0]);
        delta_lon = min(min(abs(longitude[2] - longitude[0]), abs((longitude[2] - 360.0) - longitude[0])), abs(longitude[2] - (longitude[0] - 360.0)));
        delta_lat = max(max(abs(latitude[2] - latitude[0]), abs(latitude[2] - midpoint_latitude[1])), abs(latitude[0] - midpoint_latitude[1]));
        max_lat = max(max(abs(latitude[2]), abs(latitude[0])), abs(midpoint_latitude[1]));
        //float subdivisions_20 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 1.0) * max((32.0 / 180.0) * delta_lon, 1.0);
        //float subdivisions_20 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 0.75) * max((16.0 / 90.0) * delta_lat, 1.0) * max((32.0 / 180.0) * delta_lon, 0.75);
        dist = max(0.125 * length(vec2(delta_lon, 0.5 * delta_lat)), 1.0);
        scalar = max((2.8284 / 90.0) * max_lat, 1.0);
        scalar *= 1.0;//scalar;
        float subdivisions_20 = clamp(ceil(scalar * dist), 1.0, 64.0);

        // subdivisions along v0,v1 edge
        //delta_lon = abs(longitude[0] - longitude[1]);
        delta_lon = min(min(abs(longitude[0] - longitude[1]), abs((longitude[0] - 360.0) - longitude[1])), abs(longitude[0] - (longitude[1] - 360.0)));
        delta_lat = max(max(abs(latitude[0] - latitude[1]), abs(latitude[0] - midpoint_latitude[2])), abs(latitude[1] - midpoint_latitude[2]));
        max_lat = max(max(abs(latitude[0]), abs(latitude[1])), abs(midpoint_latitude[2]));
        //float subdivisions_01 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 1.0) * max((32.0 / 180.0) * delta_lon, 1.0);
        //float subdivisions_01 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 0.75) * max((16.0 / 90.0) * delta_lat, 1.0) * max((32.0 / 180.0) * delta_lon, 0.75);
        dist = max(0.125 * length(vec2(delta_lon, 0.5 * delta_lat)), 1.0);
        scalar = max((2.8284 / 90.0) * max_lat, 1.0);
        scalar *= 1.0;//scalar;
        float subdivisions_01 = clamp(ceil(scalar * dist), 1.0, 64.0);

        // tessellation subdivisions
        float min_tessellation = min3(subdivisions_12, subdivisions_20, subdivisions_01);
        float max_tessellation = max3(subdivisions_12, subdivisions_20, subdivisions_01);
        float inner_tessellation = max(max(max_tessellation - 2.0, 1.0), min(max_tessellation, 3.0));
        gl_TessLevelOuter[0] = subdivisions_12;
        gl_TessLevelOuter[1] = subdivisions_20;
        gl_TessLevelOuter[2] = subdivisions_01;
        gl_TessLevelInner[0] = inner_tessellation;
        
        /*
        // subdivisions
        gl_TessLevelOuter[0] = 1.0; // subdivisions along v1,v2 edge
        gl_TessLevelOuter[1] = 1.0; // subdivisions along v2,v0 edge
        gl_TessLevelOuter[2] = 1.0; // subdivisions along v0,v1 edge
        gl_TessLevelInner[0] = 1.0; // internal subdivisions
        */
	}
}

float min3(float v[3]) {
    return min(min(v[0], v[1]), v[2]);
}

float min3(float v1, float v2, float v3) {
    return min(min(v1, v2), v3);
}

float max3(float v[3]) {
    return max(max(v[0], v[1]), v[2]);
}

float max3(float v1, float v2, float v3) {
    return max(max(v1, v2), v3);
}
