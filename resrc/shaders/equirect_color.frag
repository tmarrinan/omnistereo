#version 330 core

precision mediump float;

in vec3 world_position;
in vec3 world_normal;

uniform int num_of_lights;
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
    vec3 diffuse;
    vec3 specular;

    for(int i = 0; i < num_of_lights; i++) {
      //diffuse
      vec3 light_direction = normalize(light_position[i] - world_position);
      float dot_product = max(dot(world_normal, light_direction), 0.0);
      diffuse += light_color[i] * material_color * dot_product;

      //specular
      vec3 reflection_direction = normalize((2.0 * dot_product * world_normal) - light_direction);
      vec3 view_direction = normalize(camera_position - world_position);
      float dot_product2 = max(dot(reflection_direction, view_direction), 0.0);
      specular += light_color[i] *  material_specular * pow(dot_product2, material_shininess);

    }

    vec3 final_color = ambient + diffuse + specular;
    final_color.x = min(final_color.x, 1.0);
    final_color.y = min(final_color.y, 1.0);
    final_color.z = min(final_color.z, 1.0);

    FragColor = vec4(final_color, 1.0);
}
