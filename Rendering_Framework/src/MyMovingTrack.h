#pragma once

#include <glm\vec3.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <random>

class IMovingTrack
{
public:
    IMovingTrack()
    {
        this->m_direction = glm::vec3(1.0, 0.0, 0.0);
        this->setStartPosition(glm::vec3(0.0, 0.0, 0.0));

        std::random_device seedGen;
        this->m_engine = std::mt19937(seedGen());
        this->m_dist = std::normal_distribution<double>(0.0, 0.03);
    }

    virtual ~IMovingTrack() {}

    virtual void setStartPosition(const glm::vec3& startPos)
    {
        this->m_currPosition = startPos;
    }

    virtual void update()
    {
        if (this->m_frameCount > 10)
        {
            float radiansDelta = this->m_dist(this->m_engine);
            this->m_radians = this->m_radians + radiansDelta;

            this->m_radians = glm::clamp(this->m_radians, -0.1f, 0.1f);

            this->m_direction = glm::rotateY(this->m_direction, this->m_radians);
            this->m_frameCount = 0;
        }
        else
        {
            this->m_frameCount = this->m_frameCount + 1;
        }

        this->m_currPosition = this->m_currPosition + this->m_speed * this->m_direction;
        if (this->m_currPosition.x > 50.0 || this->m_currPosition.x < -50.0 || this->m_currPosition.z > 10.0 || this->
        m_currPosition.z < -250.0)
        {
            this->m_direction = -1.0f * this->m_direction;
        }
    }

public:
    virtual glm::vec3 position() const
    {
        return this->m_currPosition;
    }

    virtual glm::vec4 positionVec4() const
    {
        return glm::vec4(this->m_currPosition, 1.0);
    }

    virtual float radians() const
    {
        return this->m_radians;
    }

protected:
    glm::vec3 m_currPosition;
    glm::vec3 m_direction;
    float m_speed = 0.03;
    float m_radians = 0.0;

    std::normal_distribution<double> m_dist;
    std::mt19937 m_engine;

    int m_frameCount = 0;
};
