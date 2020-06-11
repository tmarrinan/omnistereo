#version 330 core

precision mediump float;

in vec3 world_position;
in vec3 world_normal;

uniform int num_lights;
uniform vec3 light_ambient;
uniform vec3 light_position[10];
uniform vec3 light_color[10];
uniform vec3 camera_position;
uniform vec3 material_color;      // Ka and Kd
uniform vec3 material_specular;   // Ks
uniform float material_shininess; // n

out vec4 FragColor;

void main() {
    vec3 ambient = light_ambient * material_color;
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);

    for(int i = 0; i < num_lights; i++) {
      //diffuse
      vec3 light_direction = normalize(light_position[i] - world_position);
      float n_dot_l = max(dot(world_normal, light_direction), 0.0);
      diffuse += light_color[i] * material_color * n_dot_l;

      //specular
      vec3 reflection_direction = normalize(reflect(-light_direction, world_normal));
      vec3 view_direction = normalize(camera_position - world_position);
      float r_dot_v = max(dot(reflection_direction, view_direction), 0.0);
      specular += light_color[i] *  material_specular * pow(r_dot_v, material_shininess);
    }

    vec3 final_color = min(ambient + diffuse + specular, 1.0);

    FragColor = vec4(final_color, 1.0);
}
