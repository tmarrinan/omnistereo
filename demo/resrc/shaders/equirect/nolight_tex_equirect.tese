#version 410 core

layout(triangles, equal_spacing, ccw) in;

in vec3 world_position_tesc[];
in vec3 world_normal_tesc[];
in vec2 model_texcoord_tesc[];

out vec3 world_position_tese;
out vec3 world_normal_tese;
out vec2 model_texcoord_tese;

vec2 lerp3D(vec2 v0, vec2 v1, vec2 v2);
vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2);

void main() {
    world_position_tese = lerp3D(world_position_tesc[0], world_position_tesc[1], world_position_tesc[2]);
    world_normal_tese = normalize(lerp3D(world_normal_tesc[0], world_normal_tesc[1], world_normal_tesc[2]));
    model_texcoord_tese = lerp3D(model_texcoord_tesc[0], model_texcoord_tesc[1], model_texcoord_tesc[2]);
    
    //vec4 position = vec4(lerp3D(world_position_tesc[0], world_position_tesc[1], world_position_tesc[2]), 1.0);
    //gl_Position = position;
}

vec2 lerp3D(vec2 v0, vec2 v1, vec2 v2) {
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2) {
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}
