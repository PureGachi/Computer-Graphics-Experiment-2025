#pragma once
#include <vector>
#include <numbers>
#include <glm/glm.hpp>
class Cutter{
    public:
    float radius;
    float precision;
    int width;
    int length;
    float middleX;
    float middleY;
    float middleZ;
    glm::vec3 toolPoisiton;
    std::vector<float> depthData;
    std::vector<float> ballCoords;
    std::vector<int> ballIndices;
    std::vector<int> balllineIndices;

    Cutter(float R,float P,float X,float Y,float Z,glm::vec3 TP)
        :radius(R),precision(P),middleX(X),middleY(Y),middleZ(Z),length(2 * int(Z) + 1),width(2 * int(X) + 1),depthData(length *width,5.0f),toolPoisiton(TP)
    {
    }
    void generateLowerHemisphere();
    void samplingBall();
};