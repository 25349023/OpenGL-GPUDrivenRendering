#include "Material.h"

#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

texture_data loadImg(const char* path)
{
    texture_data texture;
    int n;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* data = stbi_load(path, &texture.width, &texture.height, &n, 4);
    if (data != nullptr)
    {
        texture.data = new unsigned char[texture.width * texture.height * 4 * sizeof(unsigned char)];
        memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
        stbi_image_free(data);
    }
    return texture;
}

void Material::bindTexture(const char* path)
{
    texture_data texture = loadImg(path);
    
    glGenTextures(1, &diffuse_tex);
    glBindTexture(GL_TEXTURE_2D, diffuse_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture.width, texture.height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, texture.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
