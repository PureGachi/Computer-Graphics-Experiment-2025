#include "workpiece.hpp"


void PushData(std::vector<float> &data, int x, int z, float precion, float depth)
{
    data.push_back(float(x) * precion);
    data.push_back(depth);
    data.push_back(float(z) * precion);
}

void WorkPiece::depthToCoords()
{
    zmapCoords = {};
    for (int x = 0; x < length - 1; x++)
    {
        for (int z = 0; z < width - 1; z++)
        {
            PushData(zmapCoords, x, z, precision, getDepth(x, z));
            PushData(zmapCoords, x, z + 1, precision, getDepth(x, z + 1));
            PushData(zmapCoords, x + 1, z + 1, precision, getDepth(x + 1, z + 1));
            PushData(zmapCoords, x + 1, z, precision, getDepth(x + 1, z));
        }
    }
}

void WorkPiece::generateIndices()
{
    zmapIndices = {};
    int nums = zmapCoords.size() / 3;
    for (int i = 0; i < nums; i += 4)
    {
        zmapIndices.push_back(i);
        zmapIndices.push_back(i + 1);
        zmapIndices.push_back(i + 2);
        zmapIndices.push_back(i);
        zmapIndices.push_back(i + 2);
        zmapIndices.push_back(i + 3);
    }
}

void WorkPiece::generateLineIndices()
{
    lineIndices = {};
    int nums = zmapCoords.size() / 3;
    for (int i = 0; i < nums; i += 4)
    {
        lineIndices.push_back(i);
        lineIndices.push_back(i + 1);
        lineIndices.push_back(i + 1);
        lineIndices.push_back(i + 2);
        lineIndices.push_back(i + 2);
        lineIndices.push_back(i + 3);
        lineIndices.push_back(i + 3);
        lineIndices.push_back(i);
    }
}