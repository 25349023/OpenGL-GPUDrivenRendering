#include "Model.h"

#include <iostream>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>


Model::Model(const char* mesh_path, const char* tex_path)
{
    loadMeshes(mesh_path);
    loadMaterials(tex_path);
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

    shape.material_id = mesh->mMaterialIndex;
    shape.draw_count = mesh->mNumFaces * 3;
}

void Model::loadMaterials(const char* path)
{
    material.bindTexture(path);
}
