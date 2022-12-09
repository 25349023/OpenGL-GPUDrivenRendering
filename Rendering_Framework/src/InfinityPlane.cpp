#include "InfinityPlane.h"


InfinityPlane::InfinityPlane(const int numCascade) : NUM_CASCADE(numCascade)
{
    // initialize dynamic scene object
    const int MAX_NUM_VERTEX = numCascade * 4;
    const int MAX_NUM_INDEX = numCascade * 6;
    this->m_dynamicSO = new DynamicSceneObject(MAX_NUM_VERTEX, MAX_NUM_INDEX, false, false);
    // initialize index
    unsigned int* indexBuffer = this->m_dynamicSO->indexBuffer();
    for (unsigned int i = 0; i < numCascade; i++)
    {
        indexBuffer[i * 6 + 0] = i * 4 + 0;
        indexBuffer[i * 6 + 1] = i * 4 + 1;
        indexBuffer[i * 6 + 2] = i * 4 + 2;

        indexBuffer[i * 6 + 3] = i * 4 + 2;
        indexBuffer[i * 6 + 4] = i * 4 + 3;
        indexBuffer[i * 6 + 5] = i * 4 + 0;
    }
    this->m_dynamicSO->updateIndexBuffer(0, MAX_NUM_INDEX * 4);
    this->m_dynamicSO->setPrimitive(GL_TRIANGLES);
    this->m_dynamicSO->setPixelFunctionId(SceneManager::Instance()->m_fs_proceduralPlane);
}


InfinityPlane::~InfinityPlane() {}

void InfinityPlane::updateDataBuffer()
{
    // numCascade * 4 * 3 * 4
    this->m_dynamicSO->updateDataBuffer(0, NUM_CASCADE * 48);
}

DynamicSceneObject* InfinityPlane::sceneObject() const
{
    return this->m_dynamicSO;
}

float* InfinityPlane::cascadeDataBuffer(const int cascadeId)
{
    float* dataBuffer = this->m_dynamicSO->dataBuffer();
    return dataBuffer + cascadeId * 12;
}

void InfinityPlane::updateState(const glm::mat4& viewMat, const glm::vec3& viewPos)
{
    glm::mat4 tMat = glm::translate(glm::vec3(viewPos.x, this->m_height, viewPos.z));
    glm::mat4 viewT = glm::transpose(viewMat);
    glm::vec3 forward = -1.0f * glm::vec3(viewT[2].x, 0.0, viewT[2].z);
    glm::vec3 y(0.0, 1.0, 0.0);
    glm::vec3 x = glm::normalize(glm::cross(y, forward));

    glm::mat4 rMat;
    rMat[0] = glm::vec4(x, 0.0);
    rMat[1] = glm::vec4(y, 0.0);
    rMat[2] = glm::vec4(forward, 0.0);
    rMat[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);

    this->m_dynamicSO->setModelMat(tMat * rMat);
}
