#include "objloader.h"

ObjLoader::ObjLoader(const char *filename)
{
    _position_attrib = 0;
    _normal_attrib = 1;
    _texcoord_attrib = 2;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<Group> groups;

    readObjFile(filename, vertices, normals, texcoords, groups);
    createModels(vertices, normals, texcoords, groups);
}

ObjLoader::~ObjLoader()
{
}

void ObjLoader::readObjFile(const char *filename, std::vector<glm::vec3> &vertices,
                                       std::vector<glm::vec3> &normals,
                                       std::vector<glm::vec2> &texcoords,
                                       std::vector<Group> &groups)
{
    std::ifstream in(filename, std::ios::in);
    if (!in)
    {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        exit(1);
    }

    int current_group;
    float min_coord[3] = {9.9e12, 9.9e12, 9.9e12};
    float max_coord[3] = {-9.9e12, -9.9e12, -9.9e12};

    std::string line;
    while (std::getline(in, line))
    {
        // Read in material file
        if (line.substr(0, 7) == "mtllib ")
        {
            std::istringstream ss(line.substr(7));
            std::string mtl_filename;
            ss >> mtl_filename;

            std::string obj_filename = filename;
            size_t pos = obj_filename.rfind("/");
            std::string mtl_path = "";
            if (pos != std::string::npos)
            {
                mtl_path = obj_filename.substr(0, pos + 1);
            }
            loadMtl((mtl_path + mtl_filename).c_str());
        }
        // Read in new vertex position
        else if (line.substr(0, 2) == "v ")
        {
            std::istringstream ss(line.substr(2));
            glm::vec3 v;
            ss >> v.x;
            ss >> v.y;
            ss >> v.z;
            if (v.x < min_coord[0]) min_coord[0] = v.x;
            if (v.y < min_coord[1]) min_coord[1] = v.y;
            if (v.z < min_coord[2]) min_coord[2] = v.z;
            if (v.x > max_coord[0]) max_coord[0] = v.x;
            if (v.y > max_coord[1]) max_coord[1] = v.y;
            if (v.z > max_coord[2]) max_coord[2] = v.z;
            vertices.push_back(v);
        }
        // Read in new normal vector
        else if (line.substr(0, 3) == "vn ")
        {
            std::istringstream ss(line.substr(3));
            glm::vec3 vn;
            ss >> vn.x;
            ss >> vn.y;
            ss >> vn.z;
            normals.push_back(vn);
        }
        // Read in new texture coordinate
        else if (line.substr(0, 3) == "vt ")
        {
            std::istringstream ss(line.substr(3));
            glm::vec2 vt;
            ss >> vt.x;
            ss >> vt.y;
            texcoords.push_back(vt);
        }
        // Read in new material name (indicates new group)
        else if (line.substr(0, 7) == "usemtl ")
        {
            std::string material_name;
            std::istringstream ss(line.substr(7));
            ss >> material_name;
            int group_idx = findGroupByName(groups, material_name);
            if (group_idx >= 0)
            {
                current_group = group_idx;
            }
            else
            {
                Group new_group;
                new_group.material_name = material_name;
                groups.push_back(new_group);
                current_group = groups.size() - 1;
            }
        }
        // Read in new material name
        else if (line.substr(0, 2) == "f ")
        {
            GLuint vert1, vert2, vert3;
            GLuint norm1, norm2, norm3;
            GLuint texc1, texc2, texc3;
            Face face;
            // has textures
            if (line.find("//") == std::string::npos)
            {
                line = std::regex_replace(line, std::regex("/"), " ");
                std::istringstream ss(line.substr(2));
                ss >> vert1;
                ss >> texc1;
                ss >> norm1;
                ss >> vert2;
                ss >> texc2;
                ss >> norm2;
                ss >> vert3;
                ss >> texc3;
                ss >> norm3;
                face.vertex_indices[0] = vert1 - 1;
                face.vertex_indices[1] = vert2 - 1;
                face.vertex_indices[2] = vert3 - 1;
                face.normal_indices[0] = norm1 - 1;
                face.normal_indices[1] = norm2 - 1;
                face.normal_indices[2] = norm3 - 1;
                face.texcoord_indices[0] = texc1 - 1;
                face.texcoord_indices[1] = texc2 - 1;
                face.texcoord_indices[2] = texc3 - 1;
            }
            else {
                line = std::regex_replace(line, std::regex("//"), " ");
                std::istringstream ss(line.substr(2));
                ss >> vert1;
                ss >> norm1;
                ss >> vert2;
                ss >> norm2;
                ss >> vert3;
                ss >> norm3;
                face.vertex_indices[0] = vert1 - 1;
                face.vertex_indices[1] = vert2 - 1;
                face.vertex_indices[2] = vert3 - 1;
                face.normal_indices[0] = norm1 - 1;
                face.normal_indices[1] = norm2 - 1;
                face.normal_indices[2] = norm3 - 1;
            }
            groups[current_group].faces.push_back(face);
        }
    }

    _center.x = (min_coord[0] + max_coord[0]) / 2.0f;
    _center.y = (min_coord[1] + max_coord[1]) / 2.0f;
    _center.z = (min_coord[2] + max_coord[2]) / 2.0f;
    _size.x = max_coord[0] - min_coord[0];
    _size.y = max_coord[1] - min_coord[1];
    _size.z = max_coord[2] - min_coord[2];
}

void ObjLoader::createModels(std::vector<glm::vec3> &vertices,
                             std::vector<glm::vec3> &normals,
                             std::vector<glm::vec2> &texcoords,
                             std::vector<Group> &groups)
{
    int i, j, k;
    int face_count = 0;
    for (i = 0; i < groups.size(); i++)
    {
        Model model;
        model.material_name = groups[i].material_name;

        GLuint num_faces = groups[i].faces.size();
        GLuint num_verts = num_faces * 3;

        face_count += num_faces;

        GLfloat *model_vertices = new GLfloat[num_verts * 3];
        GLfloat *model_normals = new GLfloat[num_verts * 3];
        GLfloat *model_texcoords = new GLfloat[num_verts * 2];
        GLuint *model_indices = new GLuint[num_faces * 3];
        
        for (j = 0; j < num_faces; j++)
        {
            for (k = 0; k < 3; k++)
            {
                int vn_idx = 9 * j + 3 * k;
                int t_idx = 6 * j + 2 * k;

                glm::vec3 vertex = vertices[groups[i].faces[j].vertex_indices[k]];
                model_vertices[vn_idx] = vertex.x;
                model_vertices[vn_idx + 1] = vertex.y;
                model_vertices[vn_idx + 2] = vertex.z;

                glm::vec3 normal = normals[groups[i].faces[j].normal_indices[k]];
                model_normals[vn_idx] = normal.x;
                model_normals[vn_idx + 1] = normal.y;
                model_normals[vn_idx + 2] = normal.z;

                if (_materials[model.material_name].has_texture)
                {
                    glm::vec2 texcoord = texcoords[groups[i].faces[j].texcoord_indices[k]];
                    model_texcoords[t_idx] = texcoord.x;
                    model_texcoords[t_idx + 1] = texcoord.y;
                }
            }

            model_indices[3 * j] = 3 * j;
            model_indices[3 * j + 1] = 3 * j + 1;
            model_indices[3 * j + 2] = 3 * j + 2;
        }

        model.face_index_count = num_faces * 3;

        // Create a new Vertex Array Object
        glGenVertexArrays(1, &(model.vertex_array));
        // Set newly created Vertex Array Object as the active one we are modifying
        glBindVertexArray(model.vertex_array);

        // Create buffer to store vertex positions (3D points)
        GLuint vertex_position_buffer;
        glGenBuffers(1, &vertex_position_buffer);
        // Set newly created buffer as the active one we are modifying
        glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
        // Store array of vertex positions in the vertex_position_buffer
        glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), model_vertices, GL_STATIC_DRAW);
        // Enable position_attrib in our GPU program
        glEnableVertexAttribArray(_position_attrib);
        // Attach vertex_position_buffer to the position_attrib
        // (as 3-component floating point values)
        glVertexAttribPointer(_position_attrib, 3, GL_FLOAT, false, 0, 0);

        // Create buffer to store vertex normals (vector pointing perpendicular to surface)
        GLuint vertex_normal_buffer;
        glGenBuffers(1, &vertex_normal_buffer);
        // Set newly created buffer as the active one we are modifying
        glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
        // Store array of vertex normals in the vertex_normal_buffer
        glBufferData(GL_ARRAY_BUFFER, 3 * num_verts * sizeof(GLfloat), model_normals, GL_STATIC_DRAW);
        // Enable normal_attrib in our GPU program
        glEnableVertexAttribArray(_normal_attrib);
        // Attach vertex_normal_buffer to the normal_attrib
        // (as 3-component floating point values)
        glVertexAttribPointer(_normal_attrib, 3, GL_FLOAT, false, 0, 0);

        if (_materials[model.material_name].has_texture)
        {
            // Create buffer to store texture coordinates (2D coordinates for mapping images to the surface)
            GLuint vertex_texcoord_buffer;
            glGenBuffers(1, &vertex_texcoord_buffer);
            // Set newly created buffer as the active one we are modifying
            glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
            // Store array of vertex texture coordinates in the vertex_texcoord_buffer
            glBufferData(GL_ARRAY_BUFFER, 2 * num_verts * sizeof(GLfloat), model_texcoords, GL_STATIC_DRAW);
            // Enable texcoord_attrib in our GPU program
            glEnableVertexAttribArray(_texcoord_attrib);
            // Attach vertex_texcoord_buffer to the texcoord_attrib
            // (as 2-component floating point values)
            glVertexAttribPointer(_texcoord_attrib, 2, GL_FLOAT, false, 0, 0);
        }

        // Create buffer to store faces of the triangle
        GLuint vertex_index_buffer;
        glGenBuffers(1, &vertex_index_buffer);
        // Set newly created buffer as the active one we are modifying
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
        // Store array of vertex indices in the vertex_index_buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_faces * sizeof(GLuint), model_indices, GL_STATIC_DRAW);

        // No longer modifying our Vertex Array Object, so deselect
        glBindVertexArray(0);

        // Delete arrays
        delete[] model_vertices;
        delete[] model_normals;
        delete[] model_texcoords;
        delete[] model_indices;

        _models.push_back(model);
    }

    printf("OBJ: %d triangles\n", face_count);
}

void ObjLoader::loadMtl(const char *filename)
{
    std::ifstream in(filename, std::ios::in);
    if (!in)
    {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        exit(1);
    }

    std::string current_material;

    std::string line;
    while (std::getline(in, line))
    {
        // Read in new material name
        if (line.substr(0, 7) == "newmtl ")
        {
            std::istringstream ss(line.substr(7));
            ss >> current_material;
            Material new_mat;
            new_mat.has_texture = false;
            _materials[current_material] = new_mat;
        }
        // Read in diffuse color
        else if (line.substr(0, 3) == "Kd ")
        {
            std::istringstream ss(line.substr(3));
            ss >> _materials[current_material].color.x;
            ss >> _materials[current_material].color.y;
            ss >> _materials[current_material].color.z;
        }
        // Read in specular color
        else if (line.substr(0, 3) == "Ks ")
        {
            std::istringstream ss(line.substr(3));
            ss >> _materials[current_material].specular.x;
            ss >> _materials[current_material].specular.y;
            ss >> _materials[current_material].specular.z;
        }
        // Read in specular shininess
        else if (line.substr(0, 3) == "Ns ")
        {
            std::istringstream ss(line.substr(3));
            ss >> _materials[current_material].shininess;
        }
        // Read in diffuse texture
        else if (line.substr(0, 7) == "map_Kd ")
        {
            std::istringstream ss(line.substr(7));
            std::string img_filename;
            ss >> img_filename;

            std::string mtl_filename = filename;
            size_t pos = mtl_filename.rfind("/");
            std::string img_path = "";
            if (pos != std::string::npos)
            {
                img_path = mtl_filename.substr(0, pos + 1);
            }
            _materials[current_material].has_texture = true;
            createMaterialTexture((img_path + img_filename).c_str(), &(_materials[current_material].texture_id));
        }
    }
}

void ObjLoader::createMaterialTexture(const char *filename, GLuint *texture_id)
{
    glGenTextures(1, texture_id);
    glBindTexture(GL_TEXTURE_2D, *texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    uint8_t *pixels;
    int width, height;
    imageFileToRgba(filename, &width, &height, &pixels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    freeRgba(pixels);
}

std::vector<Model>& ObjLoader::getModelList()
{
    return _models;
}

Material& ObjLoader::getMaterial(std::string name)
{
    return _materials[name];
}

glm::vec3& ObjLoader::getCenter()
{
    return _center;
}

glm::vec3& ObjLoader::getSize()
{
    return _size;
}

int ObjLoader::findGroupByName(std::vector<Group> &groups, std::string material_name)
{
    int i;
    int group_idx = -1;
    for (i = 0; i < groups.size(); i++) {
        if (groups[i].material_name == material_name)
        {
            group_idx = i;
        }
    }
    return group_idx;
}