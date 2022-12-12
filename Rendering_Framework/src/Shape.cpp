#include "Shape.h"

void Shape::extractMeshData(const aiMesh* mesh)
{
    for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
    {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
        vertex.normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
        if (mesh->HasTextureCoords(0))
        {
            vertex.tex_coords = glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
        }
        vertices.push_back(vertex);
    }
}

void Shape::extractMeshIndices(const aiMesh* mesh)
{
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
    {
        unsigned int* idx = mesh->mFaces[f].mIndices;
        indices.insert(indices.end(), idx, idx + 3);
    }
}

void Shape::bindBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex buffer
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // index buffer
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

