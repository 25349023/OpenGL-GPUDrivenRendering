#pragma once
#include <array>
#include <vector>
#include <assimp/scene.h>

#include "Shape.h"
#include "Material.h"

struct Model
{
    const aiScene* model{};
    Shape shape{};
    Material material{};

    std::vector<unsigned int> drawCounts;
    std::vector<int> baseVertices;

    Model() = default;
    Model(const char* mesh_path, const char* tex_path);

    void loadMeshes(const char* path);
    void loadMaterials(const char* path);

    static Model merge(std::vector<Model>& models);
};

