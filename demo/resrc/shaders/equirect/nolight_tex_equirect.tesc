#version 410 core

#define M_PI 3.1415926535897932384626433832795

layout(vertices = 3) out;

in vec3 world_position_vert[];
in vec3 world_normal_vert[];
in vec2 model_texcoord_vert[];

uniform vec3 camera_position;

out vec3 world_position_tesc[];
out vec3 world_normal_tesc[];
out vec2 model_texcoord_tesc[];

const float toDegrees = 180.0 / M_PI;

float min3(float v[3]);
float max3(float v[3]);
float max3(float v[4]);

void main() {
    world_position_tesc[gl_InvocationID] = world_position_vert[gl_InvocationID];
    world_normal_tesc[gl_InvocationID] = world_normal_vert[gl_InvocationID];
    model_texcoord_tesc[gl_InvocationID] = model_texcoord_vert[gl_InvocationID];

    if (gl_InvocationID == 0) {
        // TODO: take into account camera_offset
        int i;
        vec3 vertex_direction;
        float longitude[3];
        float latitude[3];
        float midpoint_latitude[3];
        for (i = 0; i < 3; i++){
            vertex_direction = world_position_vert[i] - camera_position;
            longitude[i] = atan(vertex_direction.x, vertex_direction.z) * toDegrees;
            latitude[i] = asin(vertex_direction.y / length(vertex_direction)) * toDegrees;

            vertex_direction = (0.5 * (world_position_vert[(i + 1) % 3] + world_position_vert[(i + 2) % 3])) - camera_position;
            midpoint_latitude[i] = asin(vertex_direction.y / length(vertex_direction)) * toDegrees;
        }

        float min_lon = min3(longitude);
        float max_lon = max3(longitude);
        if (max_lon - min_lon > 180.0) { // triangle wraps around left-right edges
            for (i = 0; i < 3; i++) {
                longitude[i] = (longitude[i] > 0.0) ? longitude[i] - 360.0 : longitude[i];
            }
        }

        // subdivisions increase as edge covers a larger longitudinal span
        // subdivisions increase as the edge moves further awar from the equator
        float delta_lon;
        float max_lat;

        // 3.1748 = cube root of 32 -- check with Mark

        // subdivisions along v1,v2 edge
        delta_lon = abs(longitude[1] - longitude[2]);
        max_lat = max(max(abs(latitude[1]), abs(latitude[2])), abs(midpoint_latitude[0]));
        //float subdivisions_12 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 1.0) * max((32.0 / 180.0) * delta_lon, 1.0);
        float subdivisions_12 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 0.5) * max((32.0 / 180.0) * delta_lon, 0.5);

        // subdivisions along v2,v0 edge
        delta_lon = abs(longitude[2] - longitude[0]);
        max_lat = max(max(abs(latitude[2]), abs(latitude[0])), abs(midpoint_latitude[1]));
        //float subdivisions_20 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 1.0) * max((32.0 / 180.0) * delta_lon, 1.0);
        float subdivisions_20 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 0.75) * max((32.0 / 180.0) * delta_lon, 0.75);

        // subdivisions along v0,v1 edge
        delta_lon = abs(longitude[0] - longitude[1]);
        max_lat = max(max(abs(latitude[0]), abs(latitude[1])), abs(midpoint_latitude[2]));
        //float subdivisions_01 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 1.0) * max((32.0 / 180.0) * delta_lon, 1.0);
        float subdivisions_01 = max(pow((3.1748 / 90.0) * max_lat, 3.0), 0.75) * max((32.0 / 180.0) * delta_lon, 0.75);

        // tessellation subdivisions
        float max_tessellation = clamp(ceil(max(max(subdivisions_12, subdivisions_20), subdivisions_01)), 1.0, 64.0);
        float min_tessellation = 1.0;
        gl_TessLevelOuter[0] = clamp(ceil(subdivisions_12), 1.0, 64.0);
        gl_TessLevelOuter[1] = clamp(ceil(subdivisions_20), 1.0, 64.0);
        gl_TessLevelOuter[2] = clamp(ceil(subdivisions_01), 1.0, 64.0);
        gl_TessLevelInner[0] = max_tessellation;
        

        /*
        // subdivisions
        gl_TessLevelOuter[0] = 16.0; // subdivisions along v1,v2 edge
        gl_TessLevelOuter[1] = 16.0; // subdivisions along v2,v0 edge
        gl_TessLevelOuter[2] = 16.0; // subdivisions along v0,v1 edge
        gl_TessLevelInner[0] = 8.0; // internal subdivisions
        */
	}
}

float min3(float v[3]) {
    return min(min(v[0], v[1]), v[2]);
}

float max3(float v[3]) {
    return max(max(v[0], v[1]), v[2]);
}

float max3(float v[4]) {
    return max(max(v[0], v[1]), v[2]);
}
