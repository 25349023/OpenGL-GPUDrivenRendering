#pragma once


#include "glad/glad.h"

typedef struct _texture_data
{
    _texture_data() : width(0), height(0), data(0) {}
    int width;
    int height;
    unsigned char* data;
} texture_data;

texture_data loadImg(const char* path);

struct Material
{
    GLuint diffuse_tex{};

    Material() = default;
    
    void bindTexture(const char* path);
};
