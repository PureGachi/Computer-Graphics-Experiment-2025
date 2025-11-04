#pragma once
#include <vector>

class WorkPiece
{
public:
    int length;
    int width;
    float precision;
    std::vector<float> depthData;
    std::vector<float> zmapCoords;
    std::vector<int> zmapIndices;
    std::vector<int> lineIndices;

    WorkPiece(int l, int w, float pres)
        : length(l), width(w), precision(pres), depthData(w * l, 0.0f) {}

    // 获取某个位置的深度值（单位为毫米）
    inline float getDepth(int x, int z) const
    {
        return depthData[x * width + z];
    }

    // 设置某个位置的深度值
    inline void setDepth(int x, int z, float depth)
    {
        depthData[x * width + z] = depth;
    }

    // 将深度坐标转换成三维坐标
    void depthToCoords();

    // 根据三维坐标生成顶点索引
    void generateIndices();

    // 生成网格线条索引
    void generateLineIndices();
};