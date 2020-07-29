#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <regex>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "imgreader.h"

typedef struct Model
{
    GLuint vertex_array;
    GLuint face_index_count;
    std::string material_name;
} Model;

typedef struct Material
{
    bool has_texture;
    GLuint texture_id;
    glm::vec3 color;
    glm::vec3 specular;
    GLfloat shininess;
} Material;

typedef struct Face {
    GLuint vertex_indices[3];
    GLuint normal_indices[3];
    GLuint texcoord_indices[3];
} Face;

typedef struct Group {
    std::string material_name;
    std::vector<Face> faces;
} Group;

class ObjLoader {
private:
    std::vector<Model> _models;
    std::map<std::string, Material> _materials;
    GLuint _position_attrib;
    GLuint _normal_attrib;
    GLuint _texcoord_attrib;
    glm::vec3 _center;
    glm::vec3 _size;

    int findGroupByName(std::vector<Group> &groups, std::string material_name);

public:
    ObjLoader(const char *filename);
    ~ObjLoader();

    void readObjFile(const char *filename, std::vector<glm::vec3> &vertices,
                                           std::vector<glm::vec3> &normals,
                                           std::vector<glm::vec2> &texcoords,
                                           std::vector<Group> &groups);
    void createModels(std::vector<glm::vec3> &vertices,
                      std::vector<glm::vec3> &normals,
                      std::vector<glm::vec2> &texcoords,
                      std::vector<Group> &groups);
    void loadMtl(const char *filename);
    void createMaterialTexture(const char *filename, GLuint *texture_id);
    std::vector<Model>& getModelList();
    Material& getMaterial(std::string name);
    glm::vec3& getCenter();
    glm::vec3& getSize();
};
