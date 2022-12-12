#pragma once

#include <vector>

#include "glad/glad.h"
#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/type_ptr.hpp"

#include "assimp/scene.h"

struct Vertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec3 tex_coords{};
};

struct Shape
{
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int draw_count;

    Shape(): vao(0), vbo(0), ibo(0), draw_count(0) {}

    void extractMeshData(const aiMesh* mesh);
    void extractMeshIndices(const aiMesh* mesh);
    void bindBuffers();
};
