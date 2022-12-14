#include "Model.h"

#include <iostream>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>


Model::Model(const char* mesh_path, const char* tex_path)
{
    loadMeshes(mesh_path);
    if (tex_path)
    {
        loadMaterials(tex_path);
    }
}

void Model::loadMeshes(const char* path)
{
    model = aiImportFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices
    );

    std::cout << "There are " << model->mNumMeshes << " meshes." << std::endl;

    const aiMesh* mesh = model->mMeshes[0];

    shape.extractMeshData(mesh);
    shape.extractMeshIndices(mesh);

    shape.bindBuffers();

    shape.drawCount = mesh->mNumFaces * 3;
}

void Model::loadMaterials(const char* path)
{
    material.bindTexture(path);
}

Model Model::merge(std::vector<Model>& models)
{
    Model merged;

    // merge shapes
    auto& shape = merged.shape;
    for (int i = 0; i < models.size(); ++i)
    {
        merged.baseVertices.push_back(shape.vertices.size());

        for (Vertex& v : models[i].shape.vertices)
        {
            v.tex_coords.z = (float)i;
        }

        shape.vertices.insert(shape.vertices.end(),
            models[i].shape.vertices.begin(), models[i].shape.vertices.end());
        shape.indices.insert(shape.indices.end(),
            models[i].shape.indices.begin(), models[i].shape.indices.end());

        merged.drawCounts.push_back(models[i].shape.drawCount);
        shape.drawCount += models[i].shape.drawCount;
    }

    shape.bindBuffers();

    // merge textures
    auto& texture = merged.material.texture;
    texture.height = models[0].material.texture.height;
    texture.width = models[0].material.texture.width;

    size_t texture_size = texture.width * texture.height * 4;
    texture.data = new unsigned char[texture_size * models.size()];

    for (int i = 0; i < models.size(); ++i)
    {
        memcpy(texture.data + texture_size * i, models[i].material.texture.data, texture_size);
    }

    merged.material.bindTexture2DArray(models.size());

    return merged;
}
