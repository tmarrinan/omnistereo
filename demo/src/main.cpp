#include <iostream>
#include <cmath>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgreader.h"
#include "objloader.h"

//#define OFFSCREEN

typedef struct GlslProgram {
    GLuint program;
    std::map<std::string, GLint> uniforms;
} Program;

typedef struct App {
    bool animate;
    int animate_frame_num;
    GLuint framebuffer;
    GLuint framebuffer_texture;
    int framebuffer_width;
    int framebuffer_height;
    std::map<std::string, GlslProgram> glsl_program;
    GLuint vertex_position_attrib;
    GLuint vertex_normal_attrib;
    GLuint vertex_texcoord_attrib;
    glm::mat4 mat4_projection;
    glm::mat4 mat4_view;
    glm::mat4 mat4_model_buildings;
    glm::mat3 mat3_normal_buildings;
    glm::mat4 mat4_model_car;
    glm::mat3 mat3_normal_car;
    glm::mat4 mat4_model_skybox;
    glm::vec3 camera_position;
    glm::vec4 left_headlight;
    glm::vec4 right_headlight;
    glm::vec4 taillight;
    glm::vec4 headlight_dir;
    glm::vec4 taillight_dir;
    float camera_offset;
    float camera_angle;
    glm::vec3 car_position;
    float car_orientation;
    int num_lights;
    int num_spotlights;
    glm::vec3 light_ambient;
    GLfloat *light_position;
    GLfloat *light_color;
    GLfloat *spotlight_position;
    GLfloat *spotlight_direction;
    GLfloat *spotlight_color;
    GLfloat *spotlight_attenuation;
    GLfloat *spotlight_fov;
    ObjLoader *buildings;
    ObjLoader *car;
    ObjLoader *skybox;
    std::string save_filename;
} App;

void init(GLFWwindow *window, App &app, int width, int height);
void idle(GLFWwindow *window, App &app);
void render(GLFWwindow *window, App &app);
void onKeyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
void saveImage(const char *filename, App &app);
void loadShader(std::string key, std::string shader_filename_base, App &app);
GLint compileShader(char *source, int32_t length, GLenum type);
GLuint createShaderProgram(GLuint shaders[], uint num_shaders);
void linkShaderProgram(GLuint program);
std::string shaderTypeToString(GLenum type);
int32_t readFile(const char* filename, char** data_ptr);

int main(int argc, char **argv)
{
    int width = 1440;
    int height = 720;
    std::string save_filename = "frame";
    float camera_offset = 0.0f;
    if (argc >= 2) width = std::stoi(argv[1]);
    if (argc >= 3) height = std::stoi(argv[2]);
    if (argc >= 4) save_filename = argv[3];
    if (argc >= 5) camera_offset = std::stof(argv[4]);

    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Error: could not initialize GLFW\n");
        exit(1);
    }

    // Create a window and its OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef OFFSCREEN
    GLFWwindow *window = glfwCreateWindow(128, 64, "OBJ Loader - Demo App", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // disable v-sync
#else
    GLFWwindow *window = glfwCreateWindow(width, height, "OBJ Loader - Demo App", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // enable v-sync
#endif

    // Initialize GLAD OpenGL extension handling
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Error: could not initialize GLAD\n");
        exit(1);
    }

    // User input callbacks
    glfwSetKeyCallback(window, onKeyboard);

    // Initialize app
    App app;
    app.camera_offset = camera_offset;
    app.save_filename = save_filename;
    init(window, app, width, height);

    // Main render loop
    int frame_count = 0;
    double previous_time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    //app.animate = true;
    //while (!glfwWindowShouldClose(window) && app.animate_frame_num < 384)
    {
        // Measure speed
        double current_time = glfwGetTime();
        frame_count++;
        // Print every 2 seconds
        if (current_time - previous_time >= 2.0)
        {
            printf("%.3lf FPS (%.3lf avg frame time)\n", (double)frame_count / (current_time - previous_time), (current_time - previous_time) / (double)frame_count);

            frame_count = 0;
            previous_time = current_time;
        }

        glfwPollEvents();
        idle(window, app);
    }

    return 0;
}

void init(GLFWwindow *window, App &app, int width, int height)
{
    // save pointer to `app`
    glfwSetWindowUserPointer(window, &app);

    app.animate = false;
    app.animate_frame_num = 1;

#ifdef OFFSCREEN
    // texture to render into
    glGenTextures(1, &(app.framebuffer_texture));
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // depth buffer for offscreen framebuffer
    GLuint framebuffer_depth;
    glGenRenderbuffers(1, &framebuffer_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // offscreen framebuffer
    glGenFramebuffers(1, &(app.framebuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, app.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, app.framebuffer_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer_depth);

    // set the list of draw buffers
    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //std::cout << "FRAMEBUFFER STATUS: [" << status << "]" << std::endl;

    // save framebuffer dimensions
    app.framebuffer_width = width;
    app.framebuffer_height = height;
#else
    // save framebuffer dimensions
    glfwGetFramebufferSize(window, &(app.framebuffer_width), &(app.framebuffer_height));
    app.framebuffer = 0;
#endif

    // set viewport size and background color
    glViewport(0, 0, app.framebuffer_width, app.framebuffer_height);
    glClearColor(0.68, 0.85, 0.95, 1.0);
    //glClearColor(0.0, 0.004, 0.008, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    app.vertex_position_attrib = 0;
    app.vertex_normal_attrib = 1;
    app.vertex_texcoord_attrib = 2;

    loadShader("color", "resrc/shaders/equirect/color_equirect", app);
    loadShader("texture", "resrc/shaders/equirect/texture_equirect", app);
    loadShader("nolight_tex", "resrc/shaders/equirect/nolight_tex_equirect", app);
    loadShader("nolight_col", "resrc/shaders/equirect/nolight_col_equirect", app);

    app.buildings = new ObjLoader("resrc/models/buildings/buildings_small.obj");
    app.car = new ObjLoader("resrc/models/dodge_challenger/dodge_challenger.obj");
    app.skybox = new ObjLoader("resrc/models/skybox_night/skybox_night.obj");

    glm::vec3 buildings_center = app.buildings->getCenter();
    glm::vec3 buildings_size = app.buildings->getSize();
    float scale_factor_buildings = 1.0f / (std::max(buildings_size.x, std::max(buildings_size.y, buildings_size.z)));
    glm::vec3 buildings_location = (-1.0f * buildings_center) + glm::vec3(0.0f, buildings_size.y / 2.0f, 1.0f);

    glm::vec3 car_center = app.car->getCenter();
    glm::vec3 car_size = app.car->getSize();
    float scale_factor_car = 1.0f / (std::max(car_size.x, std::max(car_size.y, car_size.z)));
    glm::vec3 car_location = (-1.0f * car_center) + glm::vec3(0.0f, car_size.y / 2.0f, 1.0f);
    
    app.mat4_projection = glm::perspective(60.0 * (M_PI / 180.0), (double)app.framebuffer_width/(double)app.framebuffer_height, 0.01, 1500.0);
    
    app.camera_position = glm::vec3(-15.25, 1.72, 2.375);
    app.camera_angle = 0.75 * M_PI;
    glm::vec3 target = app.camera_position + glm::vec3(cos(app.camera_angle), 0.0, sin(app.camera_angle));
    app.mat4_view = glm::lookAt(app.camera_position, target, glm::vec3(0.0, 1.0, 0.0));

    // Buildings
    app.mat4_model_buildings = glm::scale(glm::mat4(1.0), glm::vec3(scale_factor_buildings, scale_factor_buildings, scale_factor_buildings));
    app.mat4_model_buildings = glm::scale(app.mat4_model_buildings, glm::vec3(75.0, 75.0, 75.0));
    app.mat4_model_buildings = glm::translate(app.mat4_model_buildings, buildings_location);

    app.mat3_normal_buildings = glm::inverse(app.mat4_model_buildings);
    app.mat3_normal_buildings = glm::transpose(app.mat3_normal_buildings);

    // Dodge Challenger
    app.car_position = glm::vec3(-23.0, 0.0, 6.5);
    app.car_orientation = 0.0;
    app.mat4_model_car = glm::translate(glm::mat4(1.0), app.car_position);
    app.mat4_model_car = glm::scale(app.mat4_model_car, glm::vec3(scale_factor_car, scale_factor_car, scale_factor_car));
    app.mat4_model_car = glm::scale(app.mat4_model_car, glm::vec3(5.03, 5.03, 5.03));
    app.mat4_model_car = glm::rotate(app.mat4_model_car, (float)(-0.5 * M_PI), glm::vec3(0.0 , 1.0, 0.0));
    app.mat4_model_car = glm::translate(app.mat4_model_car, car_location);

    app.mat3_normal_car = glm::inverse(app.mat4_model_car);
    app.mat3_normal_car = glm::transpose(app.mat3_normal_car);
    
    // Skybox
    int i;
    app.mat4_model_skybox = glm::translate(glm::mat4(1.0), app.camera_position);
    app.mat4_model_skybox = glm::scale(app.mat4_model_skybox, glm::vec3(200.0, 200.0, 200.0));
    std::vector<Model> models = app.skybox->getModelList();
    for (i = 0; i < models.size(); i++)
    {
        Material mat = app.skybox->getMaterial(models[i].material_name);
        glBindTexture(GL_TEXTURE_2D, mat.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    app.light_position = new GLfloat[3 * 24];
    app.light_color = new GLfloat[3 * 24];
    app.num_lights = 18;
    app.light_ambient = glm::vec3(0.01, 0.01, 0.01);
    app.light_position[0] = -6.6125; app.light_position[1] = 2.92; app.light_position[2] = 1.6875;
    app.light_position[3] = -6.6125; app.light_position[4] = 2.92; app.light_position[5] = 2.9375;
    app.light_position[6] = -24.625; app.light_position[7] = 2.92; app.light_position[8] = 1.6875;
    app.light_position[9] = -24.625; app.light_position[10] = 2.92; app.light_position[11] = 2.9375;
    app.light_position[12] = -17.000; app.light_position[13] = 2.92; app.light_position[14] = -4.005;
    app.light_position[15] = -17.000; app.light_position[16] = 2.92; app.light_position[17] = -5.250;
    app.light_position[18] = -8.995; app.light_position[19] = 2.92; app.light_position[20] = -4.005;
    app.light_position[21] = -8.995; app.light_position[22] = 2.92; app.light_position[23] = -5.250;
    app.light_position[24] = -2.105; app.light_position[25] = 2.92; app.light_position[26] = -4.005;
    app.light_position[27] = -2.105; app.light_position[28] = 2.92; app.light_position[29] = -5.250;
    app.light_position[30] = -22.950; app.light_position[31] = 2.92; app.light_position[32] = 9.075;
    app.light_position[33] = -22.950; app.light_position[34] = 2.92; app.light_position[35] = 10.3125;
    app.light_position[36] = -13.235; app.light_position[37] = 2.92; app.light_position[38] = 9.075;
    app.light_position[39] = -13.235; app.light_position[40] = 2.92; app.light_position[41] = 10.3125;
    app.light_position[42] = -2.5005; app.light_position[43] = 2.92; app.light_position[44] = 9.075;
    app.light_position[45] = -2.5005; app.light_position[46] = 2.92; app.light_position[47] = 10.3125;
    app.light_position[48] = -1.000; app.light_position[49] = 2.02; app.light_position[50] = 7.500;
    app.light_position[51] = -1.000; app.light_position[52] = 2.02; app.light_position[53] = -1.8755;
    for (i = 0; i < app.num_lights; i++)
    {
        app.light_color[3 * i] = 0.894;
        app.light_color[3 * i + 1] = 0.757;
        app.light_color[3 * i + 2] = 0.439;
    }

    app.spotlight_position = new GLfloat[3 * 6];
    app.spotlight_direction = new GLfloat[3 * 6];
    app.spotlight_color = new GLfloat[3 * 6];
    app.spotlight_attenuation = new GLfloat[2 * 6];
    app.spotlight_fov = new GLfloat[6];
    app.num_spotlights = 3;
    app.left_headlight = glm::vec4(-21.750, 0.695, 5.8125, 1.0);
    app.right_headlight = glm::vec4(-21.750, 0.695, 7.1875, 1.0);
    app.taillight = glm::vec4(-26.375, 0.695, 6.500, 1.0);
    app.headlight_dir = glm::vec4(0.98894, -0.14834, 0.0, 0.0);
    app.taillight_dir = glm::vec4(-1.0, 0.0, 0.0, 0.0);
    app.spotlight_position[0] = -21.750; app.spotlight_position[1] = 0.695; app.spotlight_position[2] = 5.8125;
    app.spotlight_position[3] = -21.750; app.spotlight_position[4] = 0.695; app.spotlight_position[5] = 7.1875;
    app.spotlight_position[6] = -26.375; app.spotlight_position[7] = 0.845; app.spotlight_position[8] = 6.500;
    app.spotlight_direction[0] = 0.98894; app.spotlight_direction[1] = -0.14834; app.spotlight_direction[2] = 0.0;
    app.spotlight_direction[3] = 0.98894; app.spotlight_direction[4] = -0.14834; app.spotlight_direction[5] = 0.0;
    app.spotlight_direction[6] = -1.0; app.spotlight_direction[7] = 0.0; app.spotlight_direction[8] = 0.0;
    app.spotlight_color[0] = 1.00; app.spotlight_color[1] = 0.92; app.spotlight_color[2] = 0.69;
    app.spotlight_color[3] = 1.00; app.spotlight_color[4] = 0.92; app.spotlight_color[5] = 0.69;
    app.spotlight_color[6] = 0.43; app.spotlight_color[7] = 0.01; app.spotlight_color[8] = 0.03;
    app.spotlight_attenuation[0] = 3.00; app.spotlight_attenuation[1] = 35.0;
    app.spotlight_attenuation[2] = 3.00; app.spotlight_attenuation[3] = 35.0;
    app.spotlight_attenuation[4] = 0.75; app.spotlight_attenuation[5] = 4.5;
    app.spotlight_fov[0] = 0.125 * M_PI;
    app.spotlight_fov[1] = 0.125 * M_PI;
    app.spotlight_fov[2] = 0.51 * M_PI;

    /*
    app.camera_position = glm::vec3(0.0, 0.0, 0.0);

    app.buildings = new ObjLoader("resrc/models/icosphere/icosphere.obj");
    app.mat4_model_buildings = glm::mat4(1.0);
    app.mat3_normal_buildings = glm::mat3(1.0);

    app.light_position = new GLfloat[3 * 24];
    app.light_color = new GLfloat[3 * 24];
    app.num_lights = 1;
    app.light_ambient = glm::vec3(0.05, 0.05, 0.05);
    app.light_position[0] = 0.0; app.light_position[1] = 1.0; app.light_position[2] = 0.0;
    app.light_color[0] = 1.0; app.light_color[1] = 1.0; app.light_color[2] = 1.0;
    app.num_spotlights = 0;
    */
    
    render(window, app);
}

void idle(GLFWwindow *window, App &app)
{

    if (app.animate)
    {
        //float delta_camera_angle = -0.95 * M_PI / 180.0;
        float delta_camera_angle = -0.76 * M_PI / 180.0;

        glm::vec3 new_car_position = app.car_position;
        float delta_car_orientation = 0.0;

        // drive forward
        if (app.car_position.x < -5.6192 && app.car_position.z > 6.0)
        {
            //new_car_position = app.car_position + glm::vec3(0.149, 0.0, 0.0);
            new_car_position = app.car_position + glm::vec3(0.1192, 0.0, 0.0);
        }
        // turn
        else if (app.car_orientation < M_PI)
        {
            //delta_camera_angle = -0.75 * M_PI / 180.0;
            delta_camera_angle = -0.6 * M_PI / 180.0;

            //delta_car_orientation = 0.01 * M_PI;
            delta_car_orientation = 0.008 * M_PI;
            if (app.car_orientation + delta_car_orientation > M_PI)
            {
                delta_car_orientation = M_PI - app.car_orientation;
            }

            // center (XZ): -5.5, 2.375
            // radius: 4.025
            glm::vec3 car_dir = glm::vec3(cos(app.car_orientation), 0.0, -sin(app.car_orientation));
            glm::vec3 car_back = glm::vec3(-7.5 + 4.025 * -car_dir.z, app.car_position.y, 2.375 + 4.025 * car_dir.x);
            new_car_position = car_back + (2.0f * car_dir);
        }
        // drive back
        else
        {
            //new_car_position = app.car_position + glm::vec3(-0.149, 0.0, 0.0);
            new_car_position = app.car_position + glm::vec3(-0.1192, 0.0, 0.0);
        }


        // animate camera
        app.camera_angle += delta_camera_angle;

        glm::vec3 target = app.camera_position + glm::vec3(cos(app.camera_angle), 0.0, sin(app.camera_angle));
        app.mat4_view = glm::lookAt(app.camera_position, target, glm::vec3(0.0, 1.0, 0.0));


        glm::mat4 car_transform = glm::translate(glm::mat4(1.0), new_car_position);
        car_transform = glm::rotate(car_transform, delta_car_orientation, glm::vec3(0.0 , 1.0, 0.0));
        car_transform = glm::translate(car_transform, -app.car_position);

        app.mat4_model_car = car_transform * app.mat4_model_car;

        app.car_position = new_car_position;
        app.car_orientation += delta_car_orientation;

        app.left_headlight = car_transform * app.left_headlight;
        app.spotlight_position[0] = app.left_headlight.x;
        app.spotlight_position[1] = app.left_headlight.y;
        app.spotlight_position[2] = app.left_headlight.z;

        app.right_headlight = car_transform * app.right_headlight;
        app.spotlight_position[3] = app.right_headlight.x;
        app.spotlight_position[4] = app.right_headlight.y;
        app.spotlight_position[5] = app.right_headlight.z;

        app.taillight = car_transform * app.taillight;
        app.spotlight_position[6] = app.taillight.x;
        app.spotlight_position[7] = app.taillight.y;
        app.spotlight_position[8] = app.taillight.z;

        app.headlight_dir = car_transform * app.headlight_dir;
        app.spotlight_direction[0] = app.headlight_dir.x;
        app.spotlight_direction[1] = app.headlight_dir.y;
        app.spotlight_direction[2] = app.headlight_dir.z;
        app.spotlight_direction[3] = app.headlight_dir.x;
        app.spotlight_direction[4] = app.headlight_dir.y;
        app.spotlight_direction[5] = app.headlight_dir.z;

        app.taillight_dir = car_transform * app.taillight_dir;
        app.spotlight_direction[6] = app.taillight_dir.x;
        app.spotlight_direction[7] = app.taillight_dir.y;
        app.spotlight_direction[8] = app.taillight_dir.z;

        char output_filename[64];
        sprintf(output_filename, "output/%s_%04d.ppm", app.save_filename.c_str(), app.animate_frame_num);
        //saveImage(output_filename, app);

        app.animate_frame_num++;
    }

    render(window, app);
}

void render(GLFWwindow *window, App &app)
{
    // Delete previous frame (reset both framebuffer and z-buffer)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int i;

    glBindFramebuffer(GL_FRAMEBUFFER, app.framebuffer);
    
    // Draw buildings
    std::vector<Model> models = app.buildings->getModelList();
    for (i = 0; i < models.size(); i++)
    {
        std::string program_name;
        Material mat = app.buildings->getMaterial(models[i].material_name);
        if (models[i].material_name == "15_-_Default.003")
        {
            glUseProgram(app.glsl_program["nolight_col"].program);
            program_name = "nolight_col";
            mat.color = glm::vec3(0.894, 0.757, 0.439);
        }
        else if (mat.has_texture){
            glUseProgram(app.glsl_program["texture"].program);
            program_name = "texture";

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.texture_id);
            glUniform1i(app.glsl_program[program_name].uniforms["image"], 0);
        }
        else
        {
            glUseProgram(app.glsl_program["color"].program);
            program_name = "color";
        }

        if (program_name != "nolight_col")
        {
            glUniform1i(app.glsl_program[program_name].uniforms["num_lights"], app.num_lights);
            glUniform1i(app.glsl_program[program_name].uniforms["num_spotlights"], app.num_spotlights);
            glUniform3fv(app.glsl_program[program_name].uniforms["light_ambient"], 1, glm::value_ptr(app.light_ambient));
            glUniform3fv(app.glsl_program[program_name].uniforms["light_position[0]"], app.num_lights, app.light_position);
            glUniform3fv(app.glsl_program[program_name].uniforms["light_color[0]"], app.num_lights, app.light_color);
            glUniform3fv(app.glsl_program[program_name].uniforms["spotlight_position[0]"], app.num_spotlights, app.spotlight_position);
            glUniform3fv(app.glsl_program[program_name].uniforms["spotlight_direction[0]"], app.num_spotlights, app.spotlight_direction);
            glUniform3fv(app.glsl_program[program_name].uniforms["spotlight_color[0]"], app.num_spotlights, app.spotlight_color);
            glUniform2fv(app.glsl_program[program_name].uniforms["spotlight_attenuation[0]"], app.num_spotlights, app.spotlight_attenuation);
            glUniform1fv(app.glsl_program[program_name].uniforms["spotlight_fov[0]"], app.num_spotlights, app.spotlight_fov);
        }

        glUniform3fv(app.glsl_program[program_name].uniforms["camera_position"], 1, glm::value_ptr(app.camera_position));
        glUniform1f(app.glsl_program[program_name].uniforms["camera_offset"], app.camera_offset);

        //glUniformMatrix4fv(app.glsl_program[program_name].uniforms["projection_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_projection));
        //glUniformMatrix4fv(app.glsl_program[program_name].uniforms["view_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_view));
        glUniformMatrix4fv(app.glsl_program[program_name].uniforms["model_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_model_buildings));
        if (program_name != "nolight_col") glUniformMatrix3fv(app.glsl_program[program_name].uniforms["normal_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat3_normal_buildings));

        glUniform3fv(app.glsl_program[program_name].uniforms["material_color"], 1, glm::value_ptr(mat.color));
        if (program_name != "nolight_col") glUniform3fv(app.glsl_program[program_name].uniforms["material_specular"], 1, glm::value_ptr(mat.specular));
        if (program_name != "nolight_col") glUniform1f(app.glsl_program[program_name].uniforms["material_shininess"], mat.shininess);

        glBindVertexArray(models[i].vertex_array);
        glDrawElements(GL_PATCHES, models[i].face_index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    // Draw car
    models = app.car->getModelList();
    for (i = 0; i < models.size(); i++)
    {
        std::string program_name;
        Material mat = app.car->getMaterial(models[i].material_name);
        if (models[i].material_name == "roller" || models[i].material_name == "rear_light")
        {
            glUseProgram(app.glsl_program["nolight_tex"].program);
            program_name = "nolight_tex";

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.texture_id);
            glUniform1i(app.glsl_program[program_name].uniforms["image"], 0);
        }
        else if (mat.has_texture){
            glUseProgram(app.glsl_program["texture"].program);
            program_name = "texture";

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.texture_id);
            glUniform1i(app.glsl_program[program_name].uniforms["image"], 0);
        }
        else
        {
            glUseProgram(app.glsl_program["color"].program);
            program_name = "color";
        }

        if (program_name != "nolight_tex")
        {
            glUniform1i(app.glsl_program[program_name].uniforms["num_lights"], app.num_lights);
            glUniform1i(app.glsl_program[program_name].uniforms["num_spotlights"], app.num_spotlights);
            glUniform3fv(app.glsl_program[program_name].uniforms["light_ambient"], 1, glm::value_ptr(app.light_ambient));
            glUniform3fv(app.glsl_program[program_name].uniforms["light_position[0]"], app.num_lights, app.light_position);
            glUniform3fv(app.glsl_program[program_name].uniforms["light_color[0]"], app.num_lights, app.light_color);
            glUniform3fv(app.glsl_program[program_name].uniforms["spotlight_position[0]"], app.num_spotlights, app.spotlight_position);
            glUniform3fv(app.glsl_program[program_name].uniforms["spotlight_direction[0]"], app.num_spotlights, app.spotlight_direction);
            glUniform3fv(app.glsl_program[program_name].uniforms["spotlight_color[0]"], app.num_spotlights, app.spotlight_color);
            glUniform2fv(app.glsl_program[program_name].uniforms["spotlight_attenuation[0]"], app.num_spotlights, app.spotlight_attenuation);
            glUniform1fv(app.glsl_program[program_name].uniforms["spotlight_fov[0]"], app.num_spotlights, app.spotlight_fov);
        }
        glUniform3fv(app.glsl_program[program_name].uniforms["camera_position"], 1, glm::value_ptr(app.camera_position));
        glUniform1f(app.glsl_program[program_name].uniforms["camera_offset"], app.camera_offset);

        //glUniformMatrix4fv(app.glsl_program[program_name].uniforms["projection_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_projection));
        //glUniformMatrix4fv(app.glsl_program[program_name].uniforms["view_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_view));
        glUniformMatrix4fv(app.glsl_program[program_name].uniforms["model_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_model_car));
        if (program_name != "nolight_tex") glUniformMatrix3fv(app.glsl_program[program_name].uniforms["normal_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat3_normal_car));

        if (program_name != "nolight_tex") glUniform3fv(app.glsl_program[program_name].uniforms["material_color"], 1, glm::value_ptr(mat.color));
        if (program_name != "nolight_tex") glUniform3fv(app.glsl_program[program_name].uniforms["material_specular"], 1, glm::value_ptr(mat.specular));
        if (program_name != "nolight_tex") glUniform1f(app.glsl_program[program_name].uniforms["material_shininess"], mat.shininess);

        glBindVertexArray(models[i].vertex_array);
        glDrawElements(GL_PATCHES, models[i].face_index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    // Draw Skybox
    models = app.skybox->getModelList();
    for (i = 0; i < models.size(); i++)
    {
        std::string program_name = "nolight_tex";
        glUseProgram(app.glsl_program[program_name].program);

        Material mat = app.skybox->getMaterial(models[i].material_name);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.texture_id);
        glUniform1i(app.glsl_program[program_name].uniforms["image"], 0);

        glUniform3fv(app.glsl_program[program_name].uniforms["camera_position"], 1, glm::value_ptr(app.camera_position));
        glUniform1f(app.glsl_program[program_name].uniforms["camera_offset"], app.camera_offset);
        
        //glUniformMatrix4fv(app.glsl_program[program_name].uniforms["projection_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_projection));
        //glUniformMatrix4fv(app.glsl_program[program_name].uniforms["view_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_view));
        glUniformMatrix4fv(app.glsl_program[program_name].uniforms["model_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat4_model_skybox));
        
        glBindVertexArray(models[i].vertex_array);
        glDrawElements(GL_PATCHES, models[i].face_index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    // Show frame on screen
    glfwSwapBuffers(window);
}

void onKeyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    App *app_ptr = (App*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        app_ptr->animate = !app_ptr->animate;
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        app_ptr->animate = false;

        glm::mat4 car_transform = glm::translate(glm::mat4(1.0), glm::vec3(-23.0, 0.0, 6.5));
        car_transform = glm::rotate(car_transform, -app_ptr->car_orientation, glm::vec3(0.0 , 1.0, 0.0));
        car_transform = glm::translate(car_transform, -app_ptr->car_position);

        app_ptr->mat4_model_car = car_transform * app_ptr->mat4_model_car;

        app_ptr->car_position = glm::vec3(-23.0, 0.0, 6.5);
        app_ptr->car_orientation = 0.0;
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        app_ptr->camera_position.z -= 0.125f;
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        app_ptr->camera_position.z += 0.125f;
    }
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        app_ptr->camera_position.x += 0.125f;
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        app_ptr->camera_position.x -= 0.125f;
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        app_ptr->camera_position.y -= 0.125f;
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        app_ptr->camera_position.y += 0.125f;
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        app_ptr->camera_angle -= 10.0 * M_PI / 180.0;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        app_ptr->camera_angle += 10.0 * M_PI / 180.0;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        printf("camera position: %.3f, %.3f, %.3f\n", app_ptr->camera_position.x,
                                                      app_ptr->camera_position.y,
                                                      app_ptr->camera_position.z);
    }

    glm::vec3 target = app_ptr->camera_position + glm::vec3(cos(app_ptr->camera_angle), 0.0, sin(app_ptr->camera_angle));
    app_ptr->mat4_view = glm::lookAt(app_ptr->camera_position,
                                     target,
                                     glm::vec3(0.0, 1.0, 0.0));

    app_ptr->mat4_model_skybox = glm::translate(glm::mat4(1.0), app_ptr->camera_position);
    app_ptr->mat4_model_skybox = glm::scale(app_ptr->mat4_model_skybox, glm::vec3(200.0, 200.0, 200.0));
}

void saveImage(const char *filename, App &app)
{
    uint8_t *pixels = new uint8_t[app.framebuffer_width * app.framebuffer_height * 3];
#ifdef OFFSCREEN
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
#else
    glReadPixels(0, 0, app.framebuffer_width, app.framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
#endif

    int i;
    FILE *fp = fopen(filename, "wb");
    fprintf(fp, "P6\n%d %d\n255\n", app.framebuffer_width, app.framebuffer_height);
    for (i = app.framebuffer_height - 1; i >= 0; i --) {
        fwrite(pixels + (i * app.framebuffer_width * 3), sizeof(uint8_t), app.framebuffer_width * 3, fp);
    }
    fclose(fp);
    delete[] pixels;
}

void loadShader(std::string key, std::string shader_filename_base, App &app)
{
    // Read vertex and fragment shaders from file
    char *vert_source, *frag_source, *tesc_source, *tese_source, *geom_source;
    std::string vert_filename = shader_filename_base + ".vert";
    std::string tesc_filename = shader_filename_base + ".tesc";
    std::string tese_filename = shader_filename_base + ".tese";
    std::string geom_filename = shader_filename_base + ".geom";
    std::string frag_filename = shader_filename_base + ".frag";
    int32_t vert_length = readFile(vert_filename.c_str(), &vert_source);
    int32_t tesc_length = readFile(tesc_filename.c_str(), &tesc_source);
    int32_t tese_length = readFile(tese_filename.c_str(), &tese_source);
    int32_t geom_length = readFile(geom_filename.c_str(), &geom_source);
    int32_t frag_length = readFile(frag_filename.c_str(), &frag_source);

    // Compile vetex shader
    GLuint vertex_shader = compileShader(vert_source, vert_length, GL_VERTEX_SHADER);
    // Compile tessellation control shader
    GLuint tess_ctrl_shader = compileShader(tesc_source, tesc_length, GL_TESS_CONTROL_SHADER);
    // Compile tessellation evaluation shader
    GLuint tess_eval_shader = compileShader(tese_source, tese_length, GL_TESS_EVALUATION_SHADER);
    // Compile geometry shader
    GLuint geometry_shader = compileShader(geom_source, geom_length, GL_GEOMETRY_SHADER);
    // Compile fragment shader
    GLuint fragment_shader = compileShader(frag_source, frag_length, GL_FRAGMENT_SHADER);

    // Create GPU program from the compiled vertex and fragment shaders
    GLuint shaders[5] = {vertex_shader, tess_ctrl_shader, tess_eval_shader, geometry_shader, fragment_shader};
    GlslProgram p;
    p.program = createShaderProgram(shaders, 5);

    // Specify input and output attributes for the GPU program
    glBindAttribLocation(p.program, app.vertex_position_attrib, "vertex_position");
    glBindAttribLocation(p.program, app.vertex_normal_attrib, "vertex_normal");
    glBindAttribLocation(p.program, app.vertex_texcoord_attrib, "vertex_texcoord");
    glBindFragDataLocation(p.program, 0, "FragColor");

    // Link compiled GPU program
    linkShaderProgram(p.program);

    // Get handles to uniform variables defined in the shaders
    GLint num_uniforms;
    glGetProgramiv(p.program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    int i;
    GLchar uniform_name[65];
    GLsizei max_name_length = 64;
    GLsizei name_length;
    GLint size;
    GLenum type;
    for (i = 0; i < num_uniforms; i++)
    {
        glGetActiveUniform(p.program, i, max_name_length, &name_length, &size, &type, uniform_name);
        p.uniforms[uniform_name] = glGetUniformLocation(p.program, uniform_name);
    }

    app.glsl_program[key] = p;
}

GLint compileShader(char *source, int32_t length, GLenum type)
{
    // Create a shader object
    GLint status;
    GLuint shader = glCreateShader(type);

    // Send the source to the shader object
    const char *src_bytes = const_cast<const char*>(source);
    const GLint len = length;
    glShaderSource(shader, 1, &src_bytes, &len);

    // Compile the shader program
    glCompileShader(shader);

    // Check to see if it compiled successfully
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        char *info = new char[log_length + 1];
        glGetShaderInfoLog(shader, log_length, NULL, info);
        std::string shader_type = shaderTypeToString(type);
        std::cerr << "Error: failed to compile " << shader_type << " shader:" << std::endl;
        std::cerr << info << std::endl;
        delete[] info;

        return -1;
    }

    return shader;
}

GLuint createShaderProgram(GLuint shaders[], uint num_shaders)
{
    // Create a GPU program
    GLuint program = glCreateProgram();
    
    // Attach all shaders to that program
    int i;
    for (i = 0; i < num_shaders; i++)
    {
        glAttachShader(program, shaders[i]);
    }

    return program;
}

void linkShaderProgram(GLuint program)
{
    // Link GPU program
    GLint status;
    glLinkProgram(program);

    // Check to see if it linked successfully
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        char *info = new char[log_length + 1];
        glGetProgramInfoLog(program, log_length, NULL, info);
        std::cerr << "Error: failed to link shader program" << std::endl;
        std::cerr << info << std::endl;
        delete[] info;
    }
}

std::string shaderTypeToString(GLenum type)
{
    std::string shader_type;
    switch (type)
    {
        case GL_VERTEX_SHADER:
            shader_type = "vertex";
            break;
        case GL_TESS_CONTROL_SHADER:
            shader_type = "tessellation control";
            break;
        case GL_TESS_EVALUATION_SHADER:
            shader_type = "tessellation evaluation";
            break;
        case GL_GEOMETRY_SHADER:
            shader_type = "geometry";
            break;
        case GL_FRAGMENT_SHADER:
            shader_type = "fragment";
            break;
    }
    return shader_type;
}

int32_t readFile(const char* filename, char** data_ptr)
{
    FILE *fp;
    int err = 0;
#ifdef _WIN32
    err = fopen_s(&fp, filename, "rb");
#else
    fp = fopen(filename, "rb");
#endif
    if (err != 0 || fp == NULL)
    {
        std::cerr << "Error: cannot open " << filename << std::endl;
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int32_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *data_ptr = (char*)malloc(fsize);
    size_t read = fread(*data_ptr, fsize, 1, fp);
    if (read != 1)
    {
        std::cerr << "Error: cannot read " << filename << std::endl;
        return -1;
    }

    fclose(fp);

    return fsize;
}
