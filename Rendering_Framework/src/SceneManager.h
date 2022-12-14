#pragma once

#include <glm\mat4x4.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>


#include <glad\glad.h>

// Singleton

class SceneManager
{
private:
    SceneManager() {}


public:
    virtual ~SceneManager() {}

    static SceneManager* Instance()
    {
        static SceneManager* m_instance = nullptr;
        if (m_instance == nullptr)
        {
            m_instance = new SceneManager();
        }
        return m_instance;
    }

    GLuint m_vertexHandle;
    GLuint m_normalHandle;
    GLuint m_uvHandle;
    GLuint m_offsetHandel;
    //GLint m_boneIndexHandle;
    //GLint m_weightHandle;
    //GLint m_tangentHandle;
    GLuint m_instanceMatCol0Handle;
    GLuint m_instanceMatCol1Handle;
    GLuint m_instanceMatCol2Handle;
    GLuint m_instanceMatCol3Handle;

    GLuint m_projMatHandle;
    GLuint m_viewMatHandle;
    GLuint m_modelMatHandle;
    GLuint m_instancedDrawHandle;


    GLuint m_materialHandle;


    GLuint m_gaussianWeights;

    GLuint m_lightPositionInViewHandle;

    GLuint m_fs_pixelProcessIdHandle;
    GLuint m_fs_albedoTexHandle;

    GLenum m_albedoTexUnit;
    GLenum m_normalTexUnit;

    GLenum m_albedoTexArrayUnit;


    int m_fs_textureMapping;
    int m_fs_commonProcess;
    int m_fs_gaussianX;
    int m_fs_gaussianY;
    int m_fs_proceduralPlane;
    int m_fs_pureColor;
    int m_fs_textureMappingWithTextureArray;
    int m_fs_finalPass;
    int m_fs_slime;
};
