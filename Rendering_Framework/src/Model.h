#pragma once
#include <vector>
#include <assimp/scene.h>

#include "Shape.h"
#include "Material.h"

struct Model
{
    const aiScene* model;
    Shape shape;
    Material material;
    
    Model(const char* mesh_path, const char* tex_path);

    void loadMeshes(const char* path);
    void loadMaterials(const char* path);
};
