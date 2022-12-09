#pragma once

#include <vector>
#include "Shader.h"
#include "SceneManager.h"
#include "DynamicSceneObject.h"


class SceneRenderer
{
public:
    SceneRenderer();
    virtual ~SceneRenderer();

private:
    ShaderProgram* m_shaderProgram = nullptr;
    glm::mat4 m_projMat;
    glm::mat4 m_viewMat;
    glm::vec4 m_directionalLightDir;
    int m_frameWidth;
    int m_frameHeight;

    std::vector<DynamicSceneObject*> m_sceneObjects;


public:
    void resize(const int w, const int h);
    bool initialize(const int w, const int h, ShaderProgram* shaderProgram);

    void setProjection(const glm::mat4& proj);
    void setView(const glm::mat4& view);
    void setDirectionalLightDir(const glm::vec4& dir);
    void setViewport(const int x, const int y, const int w, const int h);
    void appendObject(DynamicSceneObject* obj);

    // pipeline
public:
    void startNewFrame();
    void renderPass();

private:
    void clear(const glm::vec4& clearColor = glm::vec4(0.0, 0.0, 0.0, 1.0), const float depth = 1.0);
    bool setUpShader();
};
