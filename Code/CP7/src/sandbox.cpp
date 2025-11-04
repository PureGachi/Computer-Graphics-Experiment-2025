#pragma once
#include "camera.hpp"
#include "cutter.hpp"
#include "shader.hpp"
#include "tool.hpp"
#include "workpiece.hpp"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>

// 定义ASSETS_PATH宏（如果CMakeLists.txt中没有定义的话）
#ifndef ASSETS_PATH
#define ASSETS_PATH "src/shaders/"
#endif
glm::vec3 toolPoisiton = glm::vec3(10.0, 0.0, 10.0);
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow *window = glfwCreateWindow(width, height, "5.2Milling_with_ZMap", nullptr, nullptr);

    if (window == nullptr)
    {
        throw std::runtime_error("failed to create Window");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("failed to load glad");
    }

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glLineWidth(2.0f);
    // 初始化工件，并用一个二元函数初始化其数值
    // 长度、宽度与精度
    WorkPiece workpiece(200, 200, 0.2);
    initWorkpieceData(workpiece);
    workpiece.depthToCoords();
    workpiece.generateIndices();
    workpiece.generateLineIndices();

    // 初始化刀具
    Cutter myCutter(6, 0.2, 6.0, 4.0, 6.0, toolPoisiton);
    myCutter.generateLowerHemisphere();
    myCutter.samplingBall();

    // 读取着色器文件，并生成着色器程序
    std::string wpvertShaderPath = std::string(ASSETS_PATH) + "/workpieceshader.vert";
    std::string wpfragShaderPath = std::string(ASSETS_PATH) + "/workpieceshader.frag";
    std::string cvertShaderPath = std::string(ASSETS_PATH) + "/cutter.vert";
    std::string cfragShaderPath = std::string(ASSETS_PATH) + "/cutter.frag";
    Shader workpieceShader(wpvertShaderPath.c_str(), wpfragShaderPath.c_str());
    Shader CutterShader(cvertShaderPath.c_str(), cfragShaderPath.c_str());
    glEnable(GL_DEPTH_TEST);

    std::vector<GLuint> workGL(6);
    std::vector<GLuint> cutterGL(6);
    for (int i = 0; i < 2; i++)
    {
        glGenVertexArrays(1, &workGL[i]);
        glGenVertexArrays(1, &cutterGL[i]);
    }
    for (int i = 2; i < 6; i++)
    {
        glGenBuffers(1, &workGL[i]);
        glGenBuffers(1, &cutterGL[i]);
    }
    initWorkPieceRenderdata(workGL, workpiece);
    initCutterRenderdata(cutterGL, myCutter);

    while (!glfwWindowShouldClose(window))
    {
        // 获取间隔时间，以限定视角变动时的画面更新频率
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        // 获取当前时间，以限制工件深度更新频率
        currentUpdateTime = glfwGetTime();
        // 判断是否要开始铣削
        if (isNeedUpdate && indices < myPath.size() && currentUpdateTime - lastUpdateTime >= 0.5)
        {
            // 工件深度更新
            updateZmap(workpiece, myCutter, myPath[indices], toolPoisiton);
            workpiece.depthToCoords();
            workpiece.generateIndices();
            workpiece.generateLineIndices();
            initWorkPieceRenderdata(workGL, workpiece);
            // 铣刀位置更新
            CutterShader.use();
            cutterModelMatrix = glm::translate(cutterModelMatrix, getTranslateVec(myPath[indices], myCutter.precision));
            CutterShader.setMat4("Model", cutterModelMatrix);
            indices++;
            lastUpdateTime = currentUpdateTime;
        }
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        workpieceShader.use();
        workpieceShader.setMat4("Projection", projection);
        workpieceShader.setMat4("View", myCamera.GetViewMatrix());
        workpieceShader.setMat4("Model", ModelMatrix);
        workpieceShader.setVec3("Colors", glm::vec3(0.4, 0.4, 0.3));
        // 绘制workpiece
        glBindVertexArray(workGL[0]);
        glDrawElements(GL_TRIANGLES, workpiece.zmapIndices.size(), GL_UNSIGNED_INT, 0);

        // 绘制workplace网格
        workpieceShader.setVec3("Colors", glm::vec3(0.0, 0.0, 0.0));
        glBindVertexArray(workGL[1]);
        glDrawElements(GL_LINES, workpiece.lineIndices.size(), GL_UNSIGNED_INT, 0);

        CutterShader.use();
        CutterShader.setMat4("Projection", projection);
        CutterShader.setMat4("Model", cutterModelMatrix);
        CutterShader.setMat4("View", myCamera.GetViewMatrix());
        CutterShader.setVec3("Colors", glm::vec3(0.4, 0.4, 0.4));
        glBindVertexArray(cutterGL[0]);
        glDrawElements(GL_TRIANGLES, myCutter.ballIndices.size(), GL_UNSIGNED_INT, 0);
        CutterShader.setVec3("Colors", glm::vec3(0.0, 0.0, 0.0));
        glBindVertexArray(cutterGL[1]);
        glDrawElements(GL_LINES, myCutter.balllineIndices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
