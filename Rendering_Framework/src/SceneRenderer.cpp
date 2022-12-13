#include "SceneRenderer.h"


SceneRenderer::SceneRenderer() {}


SceneRenderer::~SceneRenderer() {}

void SceneRenderer::startNewFrame()
{
    this->m_shaderProgram->useProgram();
    this->clear();
}

void SceneRenderer::renderPass()
{
    SceneManager* manager = SceneManager::Instance();

    glm::vec4 lightDirInView = this->m_viewMat * this->m_directionalLightDir;
    glUniform4fv(manager->m_lightPositionInViewHandle, 1, glm::value_ptr(lightDirInView));
    glUniformMatrix4fv(manager->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
    glUniformMatrix4fv(manager->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));

    for (DynamicSceneObject* obj : this->m_sceneObjects)
    {
        obj->update();
    }
}

void SceneRenderer::useProgram()
{
    this->m_shaderProgram->useProgram();
}

// =======================================
void SceneRenderer::resize(const int w, const int h)
{
    this->m_frameWidth = w;
    this->m_frameHeight = h;
}

bool SceneRenderer::initialize(const int w, const int h, ShaderProgram* shaderProgram)
{
    this->m_shaderProgram = shaderProgram;

    this->resize(w, h);
    const bool flag = this->setUpShader();

    if (!flag)
    {
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    return true;
}

void SceneRenderer::setProjection(const glm::mat4& proj)
{
    this->m_projMat = proj;
}

void SceneRenderer::setView(const glm::mat4& view)
{
    this->m_viewMat = view;
}

void SceneRenderer::setDirectionalLightDir(const glm::vec4& dir)
{
    this->m_directionalLightDir = dir;
}

void SceneRenderer::setViewport(const int x, const int y, const int w, const int h)
{
    glViewport(x, y, w, h);
}

void SceneRenderer::appendObject(DynamicSceneObject* obj)
{
    this->m_sceneObjects.push_back(obj);
}

void SceneRenderer::clear(const glm::vec4& clearColor, const float depth)
{
    static const float COLOR[] = {0.0, 0.0, 0.0, 1.0};
    static const float DEPTH[] = {1.0};

    glClearBufferfv(GL_COLOR, 0, COLOR);
    glClearBufferfv(GL_DEPTH, 0, DEPTH);
}

bool SceneRenderer::setUpShader()
{
    if (this->m_shaderProgram == nullptr)
    {
        return false;
    }

    this->m_shaderProgram->useProgram();

    // shader attributes binding
    const GLuint programId = this->m_shaderProgram->programId();

    SceneManager* manager = SceneManager::Instance();
    manager->m_vertexHandle = 0;
    manager->m_normalHandle = 1;
    manager->m_uvHandle = 2;
    manager->m_offsetHandel = 3;

    // [TODO]: fix these handle index 
    manager->m_instanceMatCol0Handle = 3;
    manager->m_instanceMatCol1Handle = 4;
    manager->m_instanceMatCol2Handle = 5;
    manager->m_instanceMatCol3Handle = 6;

    manager->m_modelMatHandle = glGetUniformLocation(programId, "modelMat");
    manager->m_viewMatHandle = glGetUniformLocation(programId, "viewMat");
    manager->m_projMatHandle = glGetUniformLocation(programId, "projMat");
    manager->m_instancedDrawHandle = glGetUniformLocation(programId, "instancedDraw");

    manager->m_gaussianWeights = glGetUniformLocation(programId, "gaussianWeights");

    manager->m_lightPositionInViewHandle = glGetUniformLocation(programId, "lightPositionInView");
    manager->m_materialHandle = glGetUniformLocation(programId, "matParams");
    GLuint albedoTexHandle = glGetUniformLocation(programId, "albedoTexture");
    GLuint albedoTexArrayHandle = glGetUniformLocation(programId, "albedoTextureArray");
    //GLuint normalMapHandle = glGetUniformLocation(programId, "normalMap");

    glUniform1i(albedoTexHandle, 0);
    //glUniform1i(normalMapHandle, 1);
    glUniform1i(albedoTexArrayHandle, 1);

    manager->m_albedoTexUnit = GL_TEXTURE0;
    //manager->m_normalTexUnit = GL_TEXTURE1;
    manager->m_albedoTexArrayUnit = GL_TEXTURE1;

    manager->m_fs_pixelProcessIdHandle = glGetUniformLocation(programId, "pixelProcessId");
    manager->m_fs_commonProcess = 0;
    manager->m_fs_textureMapping = 1;
    manager->m_fs_gaussianX = 2;
    manager->m_fs_gaussianY = 3;
    manager->m_fs_proceduralPlane = 4;
    manager->m_fs_pureColor = 5;
    manager->m_fs_textureMappingWithTextureArray = 6;
    manager->m_fs_finalPass = 7;

    manager->m_fs_albedoTexHandle = glGetUniformLocation(programId, "albedoTex");

    return true;
}
