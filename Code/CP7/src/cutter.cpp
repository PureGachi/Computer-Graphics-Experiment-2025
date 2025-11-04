#include "cutter.hpp"
void Cutter::generateLowerHemisphere()
{
    // 精度：生成的纵向和横向分段数
    int numStacks = 20;
    int numSlices = 20; 
    int numRadialLayers = 10; 

    for (int i = 0; i <= numStacks; ++i)
    {
        float theta = (i * std::numbers::pi / 2) / numStacks;
        for (int j = 0; j <= numSlices; ++j)
        {
            float phi = (j * 2 * std::numbers::pi) / numSlices;

            float x = radius * sin(theta) * cos(phi);
            float y = -radius * cos(theta);
            float z = radius * sin(theta) * sin(phi);

            ballCoords.push_back((middleX + x + toolPoisiton.x) * precision);
            ballCoords.push_back((middleY + y + toolPoisiton.y) * precision);
            ballCoords.push_back((middleZ + z + toolPoisiton.z) * precision);
        }
    }

    for (int i = 0; i <= numRadialLayers; ++i)
    {
        float layerRadius = (radius * i) / numRadialLayers; 
        for (int j = 0; j <= numSlices; ++j)
        {
            float phi = (j * 2 * std::numbers::pi) / numSlices; 

            float x = layerRadius * cos(phi);
            float y = 0.0f;
            float z = layerRadius * sin(phi);

            ballCoords.push_back((middleX + x + toolPoisiton.x) * precision);
            ballCoords.push_back((middleY + y + toolPoisiton.y) * precision);
            ballCoords.push_back((middleZ + z + toolPoisiton.z) * precision);
        }
    }

    for (int i = 0; i < numStacks; ++i)
    {
        for (int j = 0; j < numSlices; ++j)
        {
            int first = i * (numSlices + 1) + j;
            int second = (i + 1) * (numSlices + 1) + j;

            ballIndices.push_back(first);
            ballIndices.push_back(second);
            ballIndices.push_back(first + 1);

            ballIndices.push_back(second);
            ballIndices.push_back(second + 1);
            ballIndices.push_back(first + 1);
        }
    }

    int baseIndex = numStacks * (numSlices + 1); // 
    for (int i = 0; i < numRadialLayers; ++i)
    {
        for (int j = 0; j < numSlices; ++j)
        {
            int first = baseIndex + i * (numSlices + 1) + j;
            int second = baseIndex + (i + 1) * (numSlices + 1) + j;

            ballIndices.push_back(first);
            ballIndices.push_back(second);
            ballIndices.push_back(first + 1);

            ballIndices.push_back(second);
            ballIndices.push_back(second + 1);
            ballIndices.push_back(first + 1);
        }
    }

    for (int i = 0; i <= numStacks; ++i)
    {
        for (int j = 0; j < numSlices; ++j)
        {
            int index = i * (numSlices + 1) + j;

            balllineIndices.push_back(index);
            balllineIndices.push_back(index + 1);

            if (i < numStacks)
            {
                balllineIndices.push_back(index);
                balllineIndices.push_back(index + (numSlices + 1));
            }
        }
    }

    for (int i = 0; i < numRadialLayers; ++i)
    {
        for (int j = 0; j < numSlices; ++j)
        {
            int first = baseIndex + i * (numSlices + 1) + j;
            int second = baseIndex + (i + 1) * (numSlices + 1) + j;

            balllineIndices.push_back(first);
            balllineIndices.push_back(first + 1);

            if (i < numRadialLayers)
            {
                balllineIndices.push_back(first);
                balllineIndices.push_back(second);
            }
        }
    }
}

void Cutter::samplingBall()
{
    for (int x = 0; x < width; x++)
    {
        for (int z = 0; z < length; z++)
        {
            float distance = sqrt((float(x) - middleX) * (float(x) - middleX) + (float(z) - middleZ) * (float(z) - middleZ));
            if (distance <= radius)
            {
                depthData[x * length + z] = -sqrt(radius * radius - distance * distance) * precision + middleY * precision;
            }
        }
    }
}