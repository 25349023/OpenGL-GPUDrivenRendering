#pragma once

#include <fstream>

class MyPoissonSample
{
public:
    MyPoissonSample() {}

    virtual ~MyPoissonSample()
    {
        delete[] this->m_positions;
    }

public:
    int m_numSample = 0;
    float* m_positions = nullptr;

public:
    static MyPoissonSample* fromFile(const std::string& fileFullpath)
    {
        // load poisson samples
        int numPoissonSample = -1;
        std::ifstream ppInput(fileFullpath, std::ios::binary);
        ppInput.read((char*)(&numPoissonSample), sizeof(int));
        float* poissonSamples = new float[numPoissonSample * 3];
        ppInput.read((char*)(poissonSamples), sizeof(float) * 3 * numPoissonSample);
        ppInput.close();

        MyPoissonSample* mps = new MyPoissonSample();
        mps->m_numSample = numPoissonSample;
        mps->m_positions = poissonSamples;

        const float SCALE = 250.0;
        for (int i = 0; i < mps->m_numSample; i++)
        {
            mps->m_positions[i * 3 + 0] = mps->m_positions[i * 3 + 0] * 250.0;
            mps->m_positions[i * 3 + 1] = 0.0;
            mps->m_positions[i * 3 + 2] = mps->m_positions[i * 3 + 2] * 250.0;
        }

        return mps;
    }
};
