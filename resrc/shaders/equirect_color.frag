#version 410 core

#define NEAR 0.01
#define FAR 1000.0

in vec3 world_position;
in vec3 world_normal;
in vec2 model_texcoord;
in vec3 model_color;
in vec3 model_center;

uniform float model_size;
uniform int num_lights;
uniform vec3 light_ambient;
uniform vec3 light_position[10];
uniform vec3 light_color[10];
uniform vec3 camera_position;
uniform float camera_offset;
uniform vec3 material_color;      // Ka and Kd
//uniform vec3 material_specular;   // Ks
//uniform float material_shininess; // n
uniform sampler2D image;

out vec4 FragColor;

void main() {
    /*
    // LIGHTING
    vec3 light_diffuse = vec3(0.0, 0.0, 0.0);
    vec3 light_specular = vec3(0.0, 0.0, 0.0);

    //vec3 material_specular = vec3(1.0, 1.0, 1.0);
    vec3 material_specular = vec3(0.0, 0.0, 0.0); // disable shiny spots
    float material_shininess = 64.0;

    for(int i = 0; i < num_lights; i++) {
        //diffuse
        vec3 light_direction = normalize(light_position[i] - world_position);
        float n_dot_l = max(dot(world_normal, light_direction), 0.0);
        light_diffuse += light_color[i] * n_dot_l;

        //specular
        vec3 reflection_direction = normalize(reflect(-light_direction, world_normal));
        vec3 view_direction = normalize(camera_position - world_position);
        float r_dot_v = max(dot(reflection_direction, view_direction), 0.0);
        light_specular += light_color[i] * pow(r_dot_v, material_shininess);
    }

    light_diffuse = min(light_diffuse, 1.0);
    light_specular = min(light_specular, 1.0);

    //vec3 final_color = min((light_ambient * material_color) + (light_diffuse * material_color) + (light_specular * material_specular), 1.0);
    vec3 final_color = min((light_ambient * model_color) + (light_diffuse * model_color) + (light_specular * material_specular), 1.0);

    FragColor = vec4(final_color, 1.0);
    */

    
    // BILLBOARD SPHERES
    vec2 norm_texcoord = (2.0 * model_texcoord) - vec2(1.0, 1.0);
    float magnitude = dot(norm_texcoord, norm_texcoord);
    if (magnitude > 1.0) {
        discard;
    }
    vec3 sphere_normal = vec3(norm_texcoord, sqrt(1.0 - magnitude));

    // TODO: calculate per billboard, not per pixel
    vec3 n = world_normal;
    vec3 u = normalize(cross(vec3(0.0, 1.0, 0.0), n));
    vec3 v = cross(n, u);
    mat3 r = mat3(u, v, n);

    sphere_normal = normalize(r * sphere_normal);
    float sphere_radius = model_size / 2.0;

    vec3 sphere_position = (sphere_normal * sphere_radius) + model_center;

    vec3 light_diffuse = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < num_lights; i++) {
        //diffuse
        vec3 light_direction = normalize(light_position[i] - sphere_position);
        float n_dot_l = max(dot(sphere_normal, light_direction), 0.0);
        light_diffuse += light_color[i] * n_dot_l;
    }

    vec3 final_color = min((light_ambient * model_color) + (light_diffuse * model_color), 1.0);

    FragColor = vec4(final_color, 1.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 cam_right = normalize(cross(sphere_position - camera_position, up));
    vec3 cam = camera_position + (camera_offset * cam_right);
    float distance = length(sphere_position - cam);
    
    gl_FragDepth = (distance - NEAR) / (FAR - NEAR);
}
