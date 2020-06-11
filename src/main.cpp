#include <iostream>
#include <cmath>
#include <map>
#include <string>
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
#include "jsobject.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

enum ModelType : uint8_t {PLANE, CUBE, SPHERE};

typedef struct Model {
    GLuint vertex_array;
    GLuint face_index_count;
} Model;

typedef struct Object {
    ModelType type;
    bool textured;
    glm::vec3 material_color;
    glm::vec3 material_specular;
    GLfloat material_shininess;
    glm::vec3 center;
    glm::vec3 size;
    GLfloat rotate_x;
    GLfloat rotate_y;
    GLfloat rotate_z;
} Object;

typedef struct Scene {
    glm::vec3 camera_pos;
    std::vector<Object*> models;
    glm::vec3 ambient_light;
    int num_lights;
    GLfloat *light_positions;
    GLfloat *light_colors;
} Scene;

typedef struct App {
    GLuint program;
    std::map<std::string,GLint> uniforms;
    GLuint vertex_position_attrib;
    GLuint vertex_normal_attrib;
    GLuint vertex_texcoord_attrib;
    Model plane_model;
    Model cube_model;
    Model sphere_model;
    glm::mat4 mat_model;
    glm::mat3 mat_normal;
    Scene scene;
} App;

void init(GLFWwindow *window, const char *scene_filename, App &app_ptr);
void initializeScene(jsvar scene_desc, Scene &scene);
void idle(GLFWwindow *window, App &app_ptr);
void render(GLFWwindow *window, App &app_ptr);
void loadShader(const char *vert_filename, const char *geom_filename, const char *frag_filename, App &app);
GLint compileShader(char *source, int32_t length, GLenum type);
GLuint createShaderProgram(GLuint vertex_shader, GLuint geometry_shader, GLuint fragment_shader);
void linkShaderProgram(GLuint program);
int32_t readFile(const char* filename, char** data_ptr);
GLuint createPlaneVao(GLuint position_attrib, GLuint normal_attrib, GLuint texcoord_attrib, GLuint *face_index_count);
GLuint createCubeVao(GLuint position_attrib, GLuint normal_attrib, GLuint texcoord_attrib, GLuint *face_index_count);
GLuint createSphereVao(GLuint position_attrib, GLuint normal_attrib, GLuint texcoord_attrib, GLuint *face_index_count);

int main(int argc, char **argv)
{
    // Read command line parameters for overall width / height
    int width = 1440;
    int height = 720;
    if (argc >= 2) width = std::stoi(argv[1]);
    if (argc >= 3) height = std::stoi(argv[2]);

    // Initialize GLFW
    if (!glfwInit())
    {
        exit(1);
    }

    // Create a window and its OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, "OmniStereo", NULL, NULL);

    // Make window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // Initialize GLAD OpenGL extension handling
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        exit(1);
    }

    // Main render loop
    App app;
    init(window, "resrc/scenes/sample_scene.json", app);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        idle(window, app);
    }

    // clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void init(GLFWwindow *window, const char *scene_filename, App &app)
{
    // Initialize OpenGL
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    std::cout << "Framebuffer size: " << w << "x" << h << std::endl;

    glViewport(0, 0, w, h);
    glClearColor(0.9, 0.9, 0.9, 1.0);
    glEnable(GL_DEPTH_TEST);

    // Initialize application
    app.vertex_position_attrib = 0;
    app.vertex_normal_attrib = 1;
    app.vertex_texcoord_attrib = 2;

    app.plane_model.vertex_array = createPlaneVao(app.vertex_position_attrib, app.vertex_normal_attrib, app.vertex_texcoord_attrib, &(app.plane_model.face_index_count));
    app.cube_model.vertex_array = createCubeVao(app.vertex_position_attrib, app.vertex_normal_attrib, app.vertex_texcoord_attrib, &(app.cube_model.face_index_count));
    app.sphere_model.vertex_array = createSphereVao(app.vertex_position_attrib, app.vertex_normal_attrib, app.vertex_texcoord_attrib, &(app.sphere_model.face_index_count));

    std::cout << "Model [plane]: " << app.plane_model.face_index_count / 3 << " triangles" << std::endl;
    std::cout << "Model [cube]: " << app.cube_model.face_index_count / 3 << " triangles" << std::endl;
    std::cout << "Model [sphere]: " << app.sphere_model.face_index_count / 3 << " triangles" << std::endl;

    initializeScene(jsobject::parseFromFile(scene_filename), app.scene);


    loadShader("resrc/shaders/equirect_color.vert", "resrc/shaders/equirect_color.geom", "resrc/shaders/equirect_color.frag", app);
}

void initializeScene(jsvar scene_desc, Scene &scene)
{
    int i;
    // Scene camera position
    scene.camera_pos = glm::vec3(scene_desc["camera_position"][0],
                                 scene_desc["camera_position"][1],
                                 scene_desc["camera_position"][2]);
    
    // Scene models
    for (i = 0; i < scene_desc["models"].length(); i++)
    {
        Object *model = new Object();
        if ((std::string)scene_desc["models"][i]["type"] == "plane")
            model->type = ModelType::PLANE;
        else if ((std::string)scene_desc["models"][i]["type"] == "cube")
            model->type = ModelType::CUBE;
        else // (std::string)scene_desc["models"][i]["type"] == "sphere"
            model->type = ModelType::SPHERE;
        model->textured = scene_desc["models"][i]["textured"];
        model->material_color = glm::vec3(scene_desc["models"][i]["material"]["color"][0],
                                          scene_desc["models"][i]["material"]["color"][1],
                                          scene_desc["models"][i]["material"]["color"][2]);
        model->material_specular = glm::vec3(scene_desc["models"][i]["material"]["specular"][0],
                                             scene_desc["models"][i]["material"]["specular"][1],
                                             scene_desc["models"][i]["material"]["specular"][2]);
        model->material_shininess = scene_desc["models"][i]["material"]["shininess"];
        model->center = glm::vec3(scene_desc["models"][i]["center"][0],
                                  scene_desc["models"][i]["center"][1],
                                  scene_desc["models"][i]["center"][2]);
        model->size = glm::vec3(scene_desc["models"][i]["size"][0],
                                scene_desc["models"][i]["size"][1],
                                scene_desc["models"][i]["size"][2]);
        model->rotate_x = scene_desc["models"][i]["rotate_x"];
        model->rotate_y = scene_desc["models"][i]["rotate_y"];
        model->rotate_z = scene_desc["models"][i]["rotate_z"];
        scene.models.push_back(model);
    }

    // Scene lights
    scene.ambient_light = glm::vec3(scene_desc["light"]["ambient"][0],
                                    scene_desc["light"]["ambient"][1],
                                    scene_desc["light"]["ambient"][2]);
    scene.num_lights = scene_desc["light"]["point_lights"].length();
    scene.light_positions = new GLfloat[3 * scene.num_lights];
    scene.light_colors = new GLfloat[3 * scene.num_lights];
    for (i = 0; i < scene.num_lights; i++)
    {
        scene.light_positions[3 * i] = scene_desc["light"]["point_lights"][i]["position"][0];
        scene.light_positions[3 * i + 1] = scene_desc["light"]["point_lights"][i]["position"][1];
        scene.light_positions[3 * i + 2] = scene_desc["light"]["point_lights"][i]["position"][2];
        scene.light_colors[3 * i] = scene_desc["light"]["point_lights"][i]["color"][0];
        scene.light_colors[3 * i + 1] = scene_desc["light"]["point_lights"][i]["color"][1];
        scene.light_colors[3 * i + 2] = scene_desc["light"]["point_lights"][i]["color"][2];
    }
}

void idle(GLFWwindow *window, App &app)
{
    render(window, app);
}

void render(GLFWwindow *window, App &app)
{
    int i;

    // Delete previous frame (reset both framebuffer and z-buffer)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw all models
    for (i = 0; i < app.scene.models.size(); i++)
    {
        // Select shader program to use
        glUseProgram(app.program);

        // Transform model to proper position, size, and orientation
        app.mat_model = glm::translate(glm::mat4(1.0), app.scene.models[i]->center);
        app.mat_model = glm::rotate(app.mat_model, glm::radians(app.scene.models[i]->rotate_z), glm::vec3(0.0, 0.0, 1.0));
        app.mat_model = glm::rotate(app.mat_model, glm::radians(app.scene.models[i]->rotate_y), glm::vec3(0.0, 1.0, 0.0));
        app.mat_model = glm::rotate(app.mat_model, glm::radians(app.scene.models[i]->rotate_x), glm::vec3(1.0, 0.0, 0.0));
        app.mat_model = glm::scale(app.mat_model, app.scene.models[i]->size);

        // Create normal matrix (based off of model matrix)
        app.mat_normal = glm::inverse(app.mat_model);
        app.mat_normal = glm::transpose(app.mat_normal);

        // Upload values to shader uniform variables
        glUniformMatrix4fv(app.uniforms["model_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat_model));
        glUniformMatrix3fv(app.uniforms["normal_matrix"], 1, GL_FALSE, glm::value_ptr(app.mat_normal));
        glUniform1i(app.uniforms["num_lights"], app.scene.num_lights);
        glUniform3fv(app.uniforms["light_ambient"], 1, glm::value_ptr(app.scene.ambient_light));
        glUniform3fv(app.uniforms["light_position[0]"], app.scene.num_lights, app.scene.light_positions);
        glUniform3fv(app.uniforms["light_color[0]"], app.scene.num_lights, app.scene.light_colors);
        glUniform3fv(app.uniforms["camera_position"], 1, glm::value_ptr(app.scene.camera_pos));
        glUniform3fv(app.uniforms["material_color"], 1, glm::value_ptr(app.scene.models[i]->material_color));
        glUniform3fv(app.uniforms["material_specular"], 1, glm::value_ptr(app.scene.models[i]->material_specular));
        glUniform1f(app.uniforms["material_shininess"], app.scene.models[i]->material_shininess);

        // Render model
        Model *model;
        switch (app.scene.models[i]->type)
        {
            case ModelType::PLANE:
                model = &(app.plane_model);
                break;
            case ModelType::CUBE:
                model = &(app.cube_model);
                break;
            case ModelType::SPHERE:
                model = &(app.sphere_model);
                break;
        }
        glBindVertexArray(model->vertex_array);
        glDrawElements(GL_TRIANGLES, model->face_index_count, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
    }

    glfwSwapBuffers(window);
}

void loadShader(const char *vert_filename, const char *geom_filename, const char *frag_filename, App &app)
{
    // Read vertex and fragment shaders from file
    char *vert_source, *geom_source, *frag_source;
    int32_t vert_length = readFile(vert_filename, &vert_source);
    int32_t geom_length = readFile(geom_filename, &geom_source);
    int32_t frag_length = readFile(frag_filename, &frag_source);

    // Compile vetex shader
    GLuint vertex_shader = compileShader(vert_source, vert_length, GL_VERTEX_SHADER);
    // Compile geometry shader
    GLuint geometry_shader = compileShader(geom_source, geom_length, GL_GEOMETRY_SHADER);
    // Compile fragment shader
    GLuint fragment_shader = compileShader(frag_source, frag_length, GL_FRAGMENT_SHADER);

    // Create GPU program from the compiled vertex and fragment shaders
    app.program = createShaderProgram(vertex_shader, geometry_shader, fragment_shader);

    // Specify input and output attributes for the GPU program
    glBindAttribLocation(app.program, app.vertex_position_attrib, "vertex_position");
    glBindAttribLocation(app.program, app.vertex_normal_attrib, "vertex_normal");
    glBindAttribLocation(app.program, app.vertex_texcoord_attrib, "vertex_texcoord");
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
        std::cerr << "Error: failed to compile " << ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment") << " shader:" << std::endl;
        std::cerr << info << std::endl;
        delete[] info;

        return -1;
    }

    return shader;
}

GLuint createShaderProgram(GLuint vertex_shader, GLuint geometry_shader, GLuint fragment_shader)
{
    // Create a GPU program
    GLuint program = glCreateProgram();
    
    // Attach the vertex, geometry, and fragment shaders to that program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, geometry_shader);
    glAttachShader(program, fragment_shader);

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
    // (as 3-component floating point values)
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
    // (as 3-component floating point values)
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
    int slices = 36;
    int stacks = 18;
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
    // (as 3-component floating point values)
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
