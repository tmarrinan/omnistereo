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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
//#include "jsobject.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//#define OFFSCREEN

typedef struct Model {
    GLuint vertex_array;
    GLuint face_index_count;
} Model;

typedef struct Scene {
    glm::vec3 camera_pos;
    Model model;
    glm::vec3 ambient_light;
    int num_lights;
    uint32_t num_points;
    GLfloat *light_positions;
    GLfloat *light_colors;
} Scene;

typedef struct App {
    GLuint framebuffer;
    GLuint framebuffer_texture;
    int framebuffer_width;
    int framebuffer_height;
    GLuint program;
    std::map<std::string,GLint> uniforms;
    GLuint vertex_position_attrib;
    GLuint vertex_normal_attrib;
    GLuint vertex_texcoord_attrib;
    GLuint point_center_attrib;
    GLuint point_color_attrib;
    GLuint point_size_attrib;
    glm::mat4 mat_model;
    glm::mat3 mat_normal;
    Scene scene;
} App;

void init(GLFWwindow *window, int width, int height, float camera_offset, const char *scene_filename, App &app_ptr);
void initializeScene(const char *scene_filename, App &app);
void initializeUniforms(float camera_offset, App &app);
void idle(GLFWwindow *window, App &app_ptr);
void render(GLFWwindow *window, App &app_ptr);
void onKeyboard(GLFWwindow *window, int key, int scancode, int action, int mods);
void saveImage(const char *filename, App &app);
void loadShader(std::string shader_filename_base, App &app);
GLint compileShader(char *source, int32_t length, GLenum type);
GLuint createShaderProgram(GLuint shaders[], uint32_t num_shaders);
void linkShaderProgram(GLuint program);
std::string shaderTypeToString(GLenum type);
int32_t readFile(const char* filename, char** data_ptr);
GLuint createPointCloudVao(GLfloat *point_centers, GLfloat *point_colors, GLfloat *point_sizes, uint32_t num_points, GLuint position_attrib,
                           GLuint normal_attrib, GLuint texcoord_attrib, GLuint point_center_attrib, GLuint point_color_attrib,
                           GLuint point_size_attrib, GLuint *face_index_count);

int main(int argc, char **argv)
{
    // Read command line parameters for overall width / height
    int width = 1440;
    int height = 720;
    std::string save_filename = "";
    float camera_offset = 0.0f;
    if (argc >= 2) width = std::stoi(argv[1]);
    if (argc >= 3) height = std::stoi(argv[2]);
    if (argc >= 4) camera_offset = std::stof(argv[3]);
    if (argc >= 5) save_filename = argv[4];

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Error: could not initialize GLFW" << std::endl;
        exit(1);
    }

    // Create a window and its OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef OFFSCREEN
    GLFWwindow *window = glfwCreateWindow(128, 64, "OmniStereo", NULL, NULL);
#else
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow *window = glfwCreateWindow(width, height, "OmniStereo", NULL, NULL);
#endif

    // Make window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    //glfwSwapInterval(0); // disable v-sync for max frame rate measurements

    // Initialize GLAD OpenGL extension handling
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Error: could not initialize GLAD" << std::endl;
        exit(1);
    }

    // User input callbacks
    glfwSetKeyCallback(window, onKeyboard);

    // Main render loop
    App app;
    //init(window, width, height, camera_offset, "resrc/ScanLook_Vehicle07_scene.pvr", app);
    init(window, width, height, camera_offset, "resrc/gromacs_full-equil.pvr", app);

    int frame_idx = 1;
    char output_filename[128];
    sprintf(output_filename, "output/%s_%05d.ppm", save_filename.c_str(), frame_idx);
    double previous_time = glfwGetTime();
    int frame_count = 0;
    render(window, app);
    //while (!glfwWindowShouldClose(window) && frame_idx <= 1050)
    while (!glfwWindowShouldClose(window))
    {
        // Save image
        if (save_filename != "")
        {
            saveImage(output_filename, app);
        }

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

        frame_idx++;
        sprintf(output_filename, "output/%s_%05d.ppm", save_filename.c_str(), frame_idx);
    }

    // clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void init(GLFWwindow *window, int width, int height, float camera_offset, const char *scene_filename, App &app)
{
    // save pointer to `app`
    glfwSetWindowUserPointer(window, &app);

    // Initialize OpenGL
#ifdef OFFSCREEN
    // texture to render into
    glGenTextures(1, &(app.framebuffer_texture));
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    app.framebuffer = 0;
    app.framebuffer_width = w;
    app.framebuffer_height = h;
#endif

    std::cout << "Framebuffer size: " << app.framebuffer_width << "x" << app.framebuffer_height << std::endl;
    
    glViewport(0, 0, app.framebuffer_width, app.framebuffer_height);
    glClearColor(0.68, 0.85, 0.95, 1.0);
    glEnable(GL_DEPTH_TEST);
#ifndef OFFSCREEN
    glEnable(GL_MULTISAMPLE);
#endif
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Initialize application
    app.vertex_position_attrib = 0;
    app.vertex_normal_attrib = 1;
    app.vertex_texcoord_attrib = 2;
    app.point_center_attrib = 3;
    app.point_color_attrib = 4;
    app.point_size_attrib = 5;

    initializeScene(scene_filename, app);

    loadShader("resrc/shaders/equirect_color", app);

    initializeUniforms(camera_offset, app);
}

void initializeScene(const char *scene_filename, App &app)
{
    const int NONE = -1;
    const int CAMERA = 0;
    const int LIGHTS = 1;
    const int POINTS = 2;

    std::cout << "Reading scene file" << std::endl;

    std::ifstream scene_file(scene_filename);
    std::string line;
    int section = NONE;
    int light_count, point_count, light_idx = 0, point_idx = 0;
    GLfloat *point_centers, *point_colors, *point_sizes;
    float x, y, z, size, red, green, blue;
    int skip = 1; // 1 out ouf every `skip` points will be rendered
    while (std::getline(scene_file, line))
    {
        // start of camera data
        if (line.length() > 0 && line[0] == 'c')
        {
            section = CAMERA;
        }
        // start of lights data
        else if (line.length() > 0 && line[0] == 'l')
        {
            std::istringstream iss(line);
            std::string name;
            iss >> name >> light_count;
            app.scene.ambient_light = glm::vec3(0.25, 0.25, 0.25);
            app.scene.num_lights = light_count;
            app.scene.light_positions = new GLfloat[3 * app.scene.num_lights];
            app.scene.light_colors = new GLfloat[3 * app.scene.num_lights];
            section = LIGHTS;
        }
        // start of points data
        else if (line.length() > 0 && line[0] == 'p')
        {
            std::istringstream iss(line);
            std::string name;
            iss >> name >> point_count;
            app.scene.num_points = (point_count - 1) / skip + 1;
            point_centers = new GLfloat[3 * app.scene.num_points];
            point_colors = new GLfloat[3 * app.scene.num_points];
            point_sizes = new GLfloat[app.scene.num_points];
            section = POINTS;
        }
        // camera data
        else if (line.length() > 0 && line[0] != '#' && section == CAMERA)
        {
            std::istringstream iss(line);
            iss >> x >> y >> z;
            app.scene.camera_pos = glm::vec3(x, y, z);
        }
        // light data
        else if (line.length() > 0 && line[0] != '#' && section == LIGHTS)
        {
            std::istringstream iss(line);
            iss >> x >> y >> z >> red >> green >> blue;
            app.scene.light_positions[3 * light_idx] = x;
            app.scene.light_positions[3 * light_idx + 1] = y;
            app.scene.light_positions[3 * light_idx + 2] = z;
            app.scene.light_colors[3 * light_idx] = red;
            app.scene.light_colors[3 * light_idx + 1] = green;
            app.scene.light_colors[3 * light_idx + 2] = blue;
            light_idx++;
        }
        // point data
        else if (line.length() > 0 && line[0] != '#' && section == POINTS)
        {
            std::istringstream iss(line);
            iss >> x >> y >> z >> size >> red >> green >> blue;
            
            if (point_idx % skip == 0)
            {
                //if (point_idx >= app.scene.num_points) std::cout << "Oops - exceeded point count" << std::endl;
                point_centers[3 * (point_idx / skip)] = x;
                point_centers[3 * (point_idx / skip) + 1] = y;
                point_centers[3 * (point_idx / skip) + 2] = z;
                point_sizes[point_idx / skip] = size;
                point_colors[3 * (point_idx / skip)] = red;
                point_colors[3 * (point_idx / skip) + 1] = green;
                point_colors[3 * (point_idx / skip) + 2] = blue;
            }
            point_idx++;
        }
    }
    std::cout << point_idx / skip << "/" << app.scene.num_points << std::endl;
    app.scene.model.vertex_array = createPointCloudVao(point_centers, point_colors, point_sizes, app.scene.num_points, app.vertex_position_attrib,
        app.vertex_normal_attrib, app.vertex_texcoord_attrib, app.point_center_attrib, app.point_color_attrib, app.point_size_attrib, &(app.scene.model.face_index_count));
    delete[] point_centers;
    delete[] point_colors;
    delete[] point_sizes;

    std::cout << "Finished" << std::endl;

    /*
    glm::vec3 camera_move_direction = glm::vec3(0.98348, -0.03766, 0.17702);
    app.scene.camera_pos = app.scene.camera_pos - (-75.0f * camera_move_direction);

    glm::vec3 f265 = app.scene.camera_pos + ((-0.2f * 265.0f) * camera_move_direction);
    printf("frame 265 location: %.4f, %.4f, %.4f\n", f265[0], f265[1], f265[2]);
    // frame 265
    // move (260.6222, -9.9799, 46.9103)

    app.scene.camera_pos = app.scene.camera_pos + ((-0.2f * 264.0f) * camera_move_direction);
    */
}

void initializeUniforms(float camera_offset, App &app)
{
    glUseProgram(app.program);

    glUniform1i(app.uniforms["num_lights"], app.scene.num_lights);
    glUniform3fv(app.uniforms["light_ambient"], 1, glm::value_ptr(app.scene.ambient_light));
    glUniform3fv(app.uniforms["light_position[0]"], app.scene.num_lights, app.scene.light_positions);
    glUniform3fv(app.uniforms["light_color[0]"], app.scene.num_lights, app.scene.light_colors);
    glUniform3fv(app.uniforms["camera_position"], 1, glm::value_ptr(app.scene.camera_pos));
    glUniform1f(app.uniforms["camera_offset"], camera_offset);

    glUseProgram(0);
}

void idle(GLFWwindow *window, App &app)
{
    // update camera
    //glm::vec3 camera_move_direction = glm::vec3(0.98348, -0.03766, 0.17702);
    //app.scene.camera_pos = app.scene.camera_pos + (-0.2f * camera_move_direction);
    //glUseProgram(app.program);
    //glUniform3fv(app.uniforms["camera_position"], 1, glm::value_ptr(app.scene.camera_pos));
    //glUseProgram(0);

    render(window, app);
}

void render(GLFWwindow *window, App &app)
{
    glBindFramebuffer(GL_FRAMEBUFFER, app.framebuffer);

    // Delete previous frame (reset both framebuffer and z-buffer)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Select shader program to use
    glUseProgram(app.program);

    // Render
    glBindVertexArray(app.scene.model.vertex_array);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElementsInstanced(GL_PATCHES, app.scene.model.face_index_count, GL_UNSIGNED_SHORT, 0, app.scene.num_points);
    glBindVertexArray(0);

    glUseProgram(0);

    glfwSwapBuffers(window);
}

void onKeyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    App *app_ptr = (App*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        saveImage("output/equirect.ppm", *app_ptr);
    }
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

void loadShader(std::string shader_filename_base, App &app)
{
    // Read vertex and fragment shaders from file
    char *vert_source, *tesc_source, *tese_source, *geom_source, *frag_source;
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
    app.program = createShaderProgram(shaders, 5);

    // Specify input and output attributes for the GPU program
    glBindAttribLocation(app.program, app.vertex_position_attrib, "vertex_position");
    glBindAttribLocation(app.program, app.vertex_normal_attrib, "vertex_normal");
    glBindAttribLocation(app.program, app.vertex_texcoord_attrib, "vertex_texcoord");
    glBindAttribLocation(app.program, app.point_center_attrib, "point_center");
    glBindAttribLocation(app.program, app.point_color_attrib, "point_color");
    glBindAttribLocation(app.program, app.point_size_attrib, "point_size");
    glBindFragDataLocation(app.program, 0, "FragColor");

    // Link compiled GPU program
    linkShaderProgram(app.program);

    // Get handles to uniform variables defined in the shaders
    GLint num_uniforms;
    glGetProgramiv(app.program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    int i;
    GLchar uniform_name[65];
    GLsizei max_name_length = 64;
    GLsizei name_length;
    GLint size;
    GLenum type;
    for (i = 0; i < num_uniforms; i++)
    {
        glGetActiveUniform(app.program, i, max_name_length, &name_length, &size, &type, uniform_name);
        app.uniforms[uniform_name] = glGetUniformLocation(app.program, uniform_name);
    }
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

GLuint createShaderProgram(GLuint shaders[], uint32_t num_shaders)
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

GLuint createPlaneVao(GLuint position_attrib, GLuint normal_attrib, GLuint texcoord_attrib, GLuint *face_index_count)
{
    // Create a new Vertex Array Object
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    // Set newly created Vertex Array Object as the active one we are modifying
    glBindVertexArray(vertex_array);

    // Create buffer to store vertex positions (3D points)
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    // Create array of 3D vertex values (each set of 3 values specifies a vertex: x, y, z)
    GLfloat vertices[12] = {
        -0.5, 0.0,  0.5,
         0.5, 0.0,  0.5,
         0.5, 0.0, -0.5,
        -0.5, 0.0, -0.5
    };
    // Store array of vertex positions in the vertex_position_buffer
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Enable position_attrib in our GPU program
    glEnableVertexAttribArray(position_attrib);
    // Attach vertex_position_buffer to the position_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store vertex normals (vector pointing perpendicular to surface)
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    // Create array of 3D vector values (each set of 3 values specifies a normalized vector: x, y, z)
    GLfloat normals[12] = {
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0
    };
    // Store array of vertex normals in the vertex_normal_buffer
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    // Enable normal_attrib in our GPU program
    glEnableVertexAttribArray(normal_attrib);
    // Attach vertex_normal_buffer to the normal_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store texture coordinates (2D coordinates for mapping images to the surface)
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    // Create array of 2D texture coordinate values (each set of 2 values specifies texture coordinate: u, v)
    GLfloat texcoords[8] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };
    // Store array of vertex texture coordinates in the vertex_texcoord_buffer
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
    // Enable texcoord_attrib in our GPU program
    glEnableVertexAttribArray(texcoord_attrib);
    // Attach vertex_texcoord_buffer to the texcoord_attrib
    // (as 2-component floating point values)
    glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, 0);

    // Create buffer to store faces of the triangle
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    // Create array of vertex indices (each set of 3 represents a triangle)
    GLushort indices[6] = {
        0, 1, 2,
        0, 2, 3
    };
    // Store array of vertex indices in the vertex_index_buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), indices, GL_STATIC_DRAW);

    // No longer modifying our Vertex Array Object, so deselect
    glBindVertexArray(0);

    // Store the number of vertices used for entire model (number of faces * 3)
    *face_index_count = 6;

    // Return created Vertex Array Object
    return vertex_array;
}

GLuint createCubeVao(GLuint position_attrib, GLuint normal_attrib, GLuint texcoord_attrib, GLuint *face_index_count)
{
    // Create a new Vertex Array Object
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    // Set newly created Vertex Array Object as the active one we are modifying
    glBindVertexArray(vertex_array);

    // Create buffer to store vertex positions (3D points)
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    // Create array of 3D vertex values (each set of 3 values specifies a vertex: x, y, z)
    GLfloat vertices[72] = {
        // Front face
        -0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,

        // Back face
         0.5, -0.5, -0.5,
        -0.5, -0.5, -0.5,
        -0.5,  0.5, -0.5,
         0.5,  0.5, -0.5,

        // Top face
        -0.5,  0.5,  0.5,
         0.5,  0.5,  0.5,
         0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5,

        // Bottom face
         0.5, -0.5,  0.5,
        -0.5, -0.5,  0.5,
        -0.5, -0.5, -0.5,
         0.5, -0.5, -0.5,

        // Right face
         0.5, -0.5,  0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
         0.5,  0.5,  0.5,

        // Left face
        -0.5, -0.5, -0.5,
        -0.5, -0.5,  0.5,
        -0.5,  0.5,  0.5,
        -0.5,  0.5, -0.5
    };
    // Store array of vertex positions in the vertex_position_buffer
    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Enable position_attrib in our GPU program
    glEnableVertexAttribArray(position_attrib);
    // Attach vertex_position_buffer to the position_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store vertex normals (vector pointing perpendicular to surface)
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    // Create array of 3D vector values (each set of 3 values specifies a normalized vector: x, y, z)
    GLfloat normals[72] = {
        // Front
        0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,

        // Back
        0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,

        // Top
        0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,

        // Bottom
        0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,

        // Right
        1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,

        // Left
        -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0
    };
    // Store array of vertex normals in the vertex_normal_buffer
    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    // Enable normal_attrib in our GPU program
    glEnableVertexAttribArray(normal_attrib);
    // Attach vertex_normal_buffer to the normal_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store texture coordinates (2D coordinates for mapping images to the surface)
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    // Create array of 2D texture coordinate values (each set of 2 values specifies texture coordinate: u, v)
    GLfloat texcoords[48] = {
        // Front
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,

        // Back
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,

        // Top
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,

        // Bottom
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,

        // Right
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,

        // Left
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0
    };
    // Store array of vertex texture coordinates in the vertex_texcoord_buffer
    glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
    // Enable texcoord_attrib in our GPU program
    glEnableVertexAttribArray(texcoord_attrib);
    // Attach vertex_texcoord_buffer to the texcoord_attrib
    // (as 2-component floating point values)
    glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, 0);

    // Create buffer to store faces of the triangle
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    // Create array of vertex indices (each set of 3 represents a triangle)
    GLushort indices[36] = {
         0,  1,  2,      0,  2,  3,   // Front
         4,  5,  6,      4,  6,  7,   // Back
         8,  9, 10,      8, 10, 11,   // Top
        12, 13, 14,     12, 14, 15,   // Bottom
        16, 17, 18,     16, 18, 19,   // Right
        20, 21, 22,     20, 22, 23    // Left
    };
    // Store array of vertex indices in the vertex_index_buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLushort), indices, GL_STATIC_DRAW);

    // No longer modifying our Vertex Array Object, so deselect
    glBindVertexArray(0);

    // Store the number of vertices used for entire model (number of faces * 3)
    *face_index_count = 36;

    // Return created Vertex Array Object
    return vertex_array;
}

GLuint createSphereVao(GLuint position_attrib, GLuint normal_attrib, GLuint texcoord_attrib, GLuint *face_index_count)
{
    // Create a new Vertex Array Object
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    // Set newly created Vertex Array Object as the active one we are modifying
    glBindVertexArray(vertex_array);

    // Calculate vertices, normals, texture coordinate, and faces
    int i, j;
    int slices = 14;
    int stacks = 7;
    int num_verts = (slices + 1) * (stacks + 1);
    int num_faces = 2 * slices * stacks;
    int vert_idx = 0;
    int face_idx = 0;
    double phi = 0;
    double delta_phi = 2.0 * M_PI / (double)slices;
    double delta_theta = -M_PI / (double)stacks;
    GLfloat *vertices = new GLfloat[num_verts * 3];
    GLfloat *normals = new GLfloat[num_verts * 3];
    GLfloat *texcoords = new GLfloat[num_verts * 2];
    for (i = 0; i <= slices; i++) {
        double cos_phi = cos(phi);
        double sin_phi = sin(phi);
        double theta = M_PI / 2.0;
        for (j = 0; j <= stacks; j++) {
            double cos_theta = cos(theta);
            double sin_theta = sin(theta);
            double x = cos_theta * cos_phi;
            double y = sin_theta;
            double z = cos_theta * -sin_phi;
            vertices[3 * vert_idx] = x / 2.0;
            vertices[3 * vert_idx + 1] = y / 2.0;
            vertices[3 * vert_idx + 2] = z / 2.0;
            normals[3 * vert_idx] = x;
            normals[3 * vert_idx + 1] = y;
            normals[3 * vert_idx + 2] = z;
            texcoords[2 * vert_idx] = (double)i / (double)slices;
            texcoords[2 * vert_idx + 1] = 1.0 - (double)j / (double)stacks;
            theta += delta_theta;
            vert_idx++;
        }
        phi += delta_phi;
    }
    GLushort *indices = new GLushort[num_faces * 3];
    for (i = 0; i < slices; i++) {
        int k1 = i * (stacks + 1);
        int k2 = (i + 1) * (stacks + 1);
        for (j = 0; j < stacks; j++) {
            indices[3 * face_idx] = k1;
            indices[3 * face_idx + 1] = k1 + 1;
            indices[3 * face_idx + 2] = k2;
            face_idx++;
            indices[3 * face_idx] = k1 + 1;
            indices[3 * face_idx + 1] = k2 + 1;
            indices[3 * face_idx + 2] = k2;
            face_idx++;
            k1++;
            k2++;
        }
    }

    // Create buffer to store vertex positions (3D points)
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    // Store array of vertex positions in the vertex_position_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Enable position_attrib in our GPU program
    glEnableVertexAttribArray(position_attrib);
    // Attach vertex_position_buffer to the position_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store vertex normals (vector pointing perpendicular to surface)
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    // Store array of vertex normals in the vertex_normal_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    // Enable normal_attrib in our GPU program
    glEnableVertexAttribArray(normal_attrib);
    // Attach vertex_normal_buffer to the normal_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store texture coordinates (2D coordinates for mapping images to the surface)
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    // Store array of vertex texture coordinates in the vertex_texcoord_buffer
    glBufferData(GL_ARRAY_BUFFER, 2 * num_verts * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
    // Enable texcoord_attrib in our GPU program
    glEnableVertexAttribArray(texcoord_attrib);
    // Attach vertex_texcoord_buffer to the texcoord_attrib
    // (as 2-component floating point values)
    glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, 0);

    // Create buffer to store faces of the triangle
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    // Store array of vertex indices in the vertex_index_buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_faces * sizeof(GLushort), indices, GL_STATIC_DRAW);

    // No longer modifying our Vertex Array Object, so deselect
    glBindVertexArray(0);

    // Delete arrays
    delete[] vertices;
    delete[] normals;
    delete[] texcoords;
    delete[] indices;

    // Store the number of vertices used for entire model (number of faces * 3)
    *face_index_count = 3 * num_faces;

    // Return created Vertex Array Object
    return vertex_array;
}

GLuint createPointCloudVao(GLfloat *point_centers, GLfloat *point_colors, GLfloat *point_sizes, uint32_t num_points, GLuint position_attrib,
                           GLuint normal_attrib, GLuint texcoord_attrib, GLuint point_center_attrib, GLuint point_color_attrib,
                           GLuint point_size_attrib, GLuint *face_index_count)
{
    // Create a new Vertex Array Object
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    // Set newly created Vertex Array Object as the active one we are modifying
    glBindVertexArray(vertex_array);

    // Calculate vertices, normals, texture coordinate, and faces
    int num_verts = 4;
    int num_faces = 2;
    GLfloat vertices[12] = {
        -0.5, -0.5,  0.0,
         0.5, -0.5,  0.0,
         0.5,  0.5,  0.0,
        -0.5,  0.5,  0.0
    };
    GLfloat normals[12] = {
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0
    };
    GLfloat texcoords[8] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };
    GLushort indices[6] = {
        0, 1, 2,
        0, 2, 3
    };
    /*
    int i, j;
    int slices = 14;
    int stacks = 7;
    int num_verts = (slices + 1) * (stacks + 1);
    int num_faces = 2 * slices * stacks;
    int vert_idx = 0;
    int face_idx = 0;
    double phi = 0;
    double delta_phi = 2.0 * M_PI / (double)slices;
    double delta_theta = -M_PI / (double)stacks;
    GLfloat *vertices = new GLfloat[num_verts * 3];
    GLfloat *normals = new GLfloat[num_verts * 3];
    GLfloat *texcoords = new GLfloat[num_verts * 2];
    for (i = 0; i <= slices; i++) {
        double cos_phi = cos(phi);
        double sin_phi = sin(phi);
        double theta = M_PI / 2.0;
        for (j = 0; j <= stacks; j++) {
            double cos_theta = cos(theta);
            double sin_theta = sin(theta);
            double x = cos_theta * cos_phi;
            double y = sin_theta;
            double z = cos_theta * -sin_phi;
            vertices[3 * vert_idx] = x / 2.0;
            vertices[3 * vert_idx + 1] = y / 2.0;
            vertices[3 * vert_idx + 2] = z / 2.0;
            normals[3 * vert_idx] = x;
            normals[3 * vert_idx + 1] = y;
            normals[3 * vert_idx + 2] = z;
            texcoords[2 * vert_idx] = (double)i / (double)slices;
            texcoords[2 * vert_idx + 1] = 1.0 - (double)j / (double)stacks;
            theta += delta_theta;
            vert_idx++;
        }
        phi += delta_phi;
    }
    GLushort *indices = new GLushort[num_faces * 3];
    for (i = 0; i < slices; i++) {
        int k1 = i * (stacks + 1);
        int k2 = (i + 1) * (stacks + 1);
        for (j = 0; j < stacks; j++) {
            indices[3 * face_idx] = k1;
            indices[3 * face_idx + 1] = k1 + 1;
            indices[3 * face_idx + 2] = k2;
            face_idx++;
            indices[3 * face_idx] = k1 + 1;
            indices[3 * face_idx + 1] = k2 + 1;
            indices[3 * face_idx + 2] = k2;
            face_idx++;
            k1++;
            k2++;
        }
    }
    */

    // Create buffer to store vertex positions (3D points)
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    // Store array of vertex positions in the vertex_position_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Enable position_attrib in our GPU program
    glEnableVertexAttribArray(position_attrib);
    // Attach vertex_position_buffer to the position_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store vertex normals (vector pointing perpendicular to surface)
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    // Store array of vertex normals in the vertex_normal_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    // Enable normal_attrib in our GPU program
    glEnableVertexAttribArray(normal_attrib);
    // Attach vertex_normal_buffer to the normal_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store texture coordinates (2D coordinates for mapping images to the surface)
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    // Store array of vertex texture coordinates in the vertex_texcoord_buffer
    glBufferData(GL_ARRAY_BUFFER, 2 * num_verts * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
    // Enable texcoord_attrib in our GPU program
    glEnableVertexAttribArray(texcoord_attrib);
    // Attach vertex_texcoord_buffer to the texcoord_attrib
    // (as 2-component floating point values)
    glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, 0);

    // Create buffer to store faces of the triangle
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    // Store array of vertex indices in the vertex_index_buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_faces * sizeof(GLushort), indices, GL_STATIC_DRAW);


    // Point cloud data
    // Create buffer to store point center positions
    GLuint point_center_buffer;
    glGenBuffers(1, &point_center_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, point_center_buffer);
    // Store array of point centers in the point_center_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_points * sizeof(GLfloat), point_centers, GL_STATIC_DRAW);
    // Enable point_center_attrib in our GPU program
    glEnableVertexAttribArray(point_center_attrib);
    // Attach point_center_buffer to the point_center_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(point_center_attrib, 3, GL_FLOAT, false, 0, 0);
    // advance one vertex attribute per instance
    glVertexAttribDivisor(point_center_attrib, 1);

    // Create buffer to store point colors
    GLuint point_color_buffer;
    glGenBuffers(1, &point_color_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, point_color_buffer);
    // Store array of point colors in the point_color_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_points * sizeof(GLfloat), point_colors, GL_STATIC_DRAW);
    // Enable point_color_attrib in our GPU program
    glEnableVertexAttribArray(point_color_attrib);
    // Attach point_color_buffer to the point_color_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(point_color_attrib, 3, GL_FLOAT, false, 0, 0);
    // advance one vertex attribute per instance
    glVertexAttribDivisor(point_color_attrib, 1);

    // Create buffer to store point sizes
    GLuint point_size_buffer;
    glGenBuffers(1, &point_size_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, point_size_buffer);
    // Store array of point sizes in the point_size_buffer
    glBufferData(GL_ARRAY_BUFFER, num_points * sizeof(GLfloat), point_sizes, GL_STATIC_DRAW);
    // Enable point_size_attrib in our GPU program
    glEnableVertexAttribArray(point_size_attrib);
    // Attach point_size_buffer to the point_size_attrib
    // (as floating point values)
    glVertexAttribPointer(point_size_attrib, 1, GL_FLOAT, false, 0, 0);
    // advance one vertex attribute per instance
    glVertexAttribDivisor(point_size_attrib, 1);


    // No longer modifying our Vertex Array Object, so deselect
    glBindVertexArray(0);

    // Delete arrays
    //delete[] vertices;
    //delete[] normals;
    //delete[] texcoords;
    //delete[] indices;

    // Store the number of vertices used for entire model (number of faces * 3)
    *face_index_count = 3 * num_faces;

    // Return created Vertex Array Object
    return vertex_array;
}

/*
void addSphereToModel(float cx, float cy, float cz, float size, float red, float green, float blue, int slices, int stacks, int sphere_num, GLfloat *vertices, GLfloat *normals, GLfloat *colors, GLuint *indices)
{
    int i, j;
    int num_verts = (slices + 1) * (stacks + 1);
    int num_faces = 2 * slices * stacks;
    int vert_offset = num_verts * 3 * sphere_num;
    int face_offset = num_faces * 3 * sphere_num;
    int vert_idx = 0;
    int face_idx = 0;
    double phi = 0;
    double delta_phi = 2.0 * M_PI / (double)slices;
    double delta_theta = -M_PI / (double)stacks;

    for (i = 0; i <= slices; i++) {
        double cos_phi = cos(phi);
        double sin_phi = sin(phi);
        double theta = M_PI / 2.0;
        for (j = 0; j <= stacks; j++) {
            double cos_theta = cos(theta);
            double sin_theta = sin(theta);
            double x = cos_theta * cos_phi;
            double y = sin_theta;
            double z = cos_theta * -sin_phi;
            vertices[vert_offset + 3 * vert_idx] = (x / 2.0) * size + cx;
            vertices[vert_offset + 3 * vert_idx + 1] = (y / 2.0) * size + cy;
            vertices[vert_offset + 3 * vert_idx + 2] = (z / 2.0) * size + cz;
            normals[vert_offset + 3 * vert_idx] = x;
            normals[vert_offset + 3 * vert_idx + 1] = y;
            normals[vert_offset + 3 * vert_idx + 2] = z;
            colors[vert_offset + 3 * vert_idx] = red;
            colors[vert_offset + 3 * vert_idx + 1] = green;
            colors[vert_offset + 3 * vert_idx + 2] = blue;
            //texcoords[2 * vert_idx] = (double)i / (double)slices;
            //texcoords[2 * vert_idx + 1] = 1.0 - (double)j / (double)stacks;
            theta += delta_theta;
            vert_idx++;
        }
        phi += delta_phi;
    }
    for (i = 0; i < slices; i++) {
        int k1 = (vert_offset / 3) + i * (stacks + 1);
        int k2 = (vert_offset / 3) + (i + 1) * (stacks + 1);
        for (j = 0; j < stacks; j++) {
            indices[face_offset + 3 * face_idx] = k1;
            indices[face_offset + 3 * face_idx + 1] = k1 + 1;
            indices[face_offset + 3 * face_idx + 2] = k2;
            face_idx++;
            indices[face_offset + 3 * face_idx] = k1 + 1;
            indices[face_offset + 3 * face_idx + 1] = k2 + 1;
            indices[face_offset + 3 * face_idx + 2] = k2;
            face_idx++;
            k1++;
            k2++;


        }
    }
}

GLuint createVertexArrayObject(int num_verts, int num_faces, GLuint position_attrib, GLuint normal_attrib, GLuint color_attrib, GLfloat *vertices, GLfloat *normals, GLfloat *colors, GLuint *indices)
{
    // Create a new Vertex Array Object
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    // Set newly created Vertex Array Object as the active one we are modifying
    glBindVertexArray(vertex_array);

    // Create buffer to store vertex positions (3D points)
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    // Store array of vertex positions in the vertex_position_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Enable position_attrib in our GPU program
    glEnableVertexAttribArray(position_attrib);
    // Attach vertex_position_buffer to the position_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store vertex normals (vector pointing perpendicular to surface)
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    // Store array of vertex normals in the vertex_normal_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    // Enable normal_attrib in our GPU program
    glEnableVertexAttribArray(normal_attrib);
    // Attach vertex_normal_buffer to the normal_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store colors (RGB)
    GLuint vertex_color_buffer;
    glGenBuffers(1, &vertex_color_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ARRAY_BUFFER, vertex_color_buffer);
    // Store array of vertex texture coordinates in the vertex_color_buffer
    glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    // Enable color_attrib in our GPU program
    glEnableVertexAttribArray(color_attrib);
    // Attach vertex_color_buffer to the color_attrib
    // (as 3-component floating point values)
    glVertexAttribPointer(color_attrib, 3, GL_FLOAT, false, 0, 0);

    // Create buffer to store faces of the triangle
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    // Set newly created buffer as the active one we are modifying
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    // Store array of vertex indices in the vertex_index_buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_faces * sizeof(GLuint), indices, GL_STATIC_DRAW);

    // No longer modifying our Vertex Array Object, so deselect
    glBindVertexArray(0);

    // Return created Vertex Array Object
    return vertex_array;
}
*/
