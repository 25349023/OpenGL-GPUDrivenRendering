#pragma once

#include "DynamicSceneObject.h"

class InfinityPlane
{
public:
    InfinityPlane(const int numCascade);
    virtual ~InfinityPlane();

    DynamicSceneObject* sceneObject() const;

public:
    float* cascadeDataBuffer(const int cascadeId);
    void updateDataBuffer();
    void updateState(const glm::mat4& viewMat, const glm::vec3& viewPos);

private:
    const int NUM_CASCADE;
    float m_height = 0.0;
    DynamicSceneObject* m_dynamicSO;
};
