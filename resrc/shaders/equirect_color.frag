#version 410 core

in vec3 world_position;
in vec3 world_normal;
in vec3 model_color;

uniform int num_lights;
uniform vec3 light_ambient;
uniform vec3 light_position[10];
uniform vec3 light_color[10];
uniform vec3 camera_position;
//uniform vec3 material_color;      // Ka and Kd
//uniform vec3 material_specular;   // Ks
//uniform float material_shininess; // n

out vec4 FragColor;

void main() {
    vec3 light_diffuse = vec3(0.0, 0.0, 0.0);
    vec3 light_specular = vec3(0.0, 0.0, 0.0);

    vec3 material_specular = vec3(1.0, 1.0, 1.0);
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
}
