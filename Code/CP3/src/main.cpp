#include <glad/glad.h>
//需要注意glad需要在GLFW之前引入
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Utils.h"
#include <iostream>

// 窗口尺寸
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// 函数声明
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW初始化失败" << std::endl;
        return -1;
    }

    // 配置GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FrameBuffer模糊效果", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "GLFW窗口创建失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD初始化失败" << std::endl;
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 构建和编译着色器程序
    // 场景着色器：用于渲染三角形到FrameBuffer
    unsigned int sceneShader = Utils::createShaderProgram("shaders/scene_vertex.glsl", "shaders/scene_fragment.glsl");
    // 屏幕着色器：用于应用模糊效果
    unsigned int screenShader =
        Utils::createShaderProgram("shaders/screen_vertex.glsl", "shaders/screen_fragment.glsl");

    if (sceneShader == 0 || screenShader == 0)
    {
        std::cerr << "着色器程序创建失败" << std::endl;
        return -1;
    }

    // 三角形顶点数据
    float triangleVertices[] = {
        // 位置
        0.0f,  0.5f,  0.0f, // 顶部
        -0.5f, -0.5f, 0.0f, // 左下
        0.5f,  -0.5f, 0.0f  // 右下
    };

    // 全屏四边形顶点数据（用于渲染帧缓冲纹理）
    float quadVertices[] = {
        // 位置        // 纹理坐标
        -1.0f, 1.0f,  0.0f, 1.0f, // 左上
        -1.0f, -1.0f, 0.0f, 0.0f, // 左下
        1.0f,  -1.0f, 1.0f, 0.0f, // 右下

        -1.0f, 1.0f,  0.0f, 1.0f, // 左上
        1.0f,  -1.0f, 1.0f, 0.0f, // 右下
        1.0f,  1.0f,  1.0f, 1.0f  // 右上
    };

    // 设置三角形VAO
    unsigned int triangleVAO, triangleVBO;
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);

    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 设置屏幕四边形VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 创建帧缓冲对象
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 创建纹理附件
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // 创建渲染缓冲对象用于深度和模板测试
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // 检查帧缓冲是否完整
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "帧缓冲不完整!" << std::endl;
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 设置屏幕着色器的纹理采样器
    glUseProgram(screenShader);
    glUniform1i(glGetUniformLocation(screenShader, "screenTexture"), 0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 输入处理
        processInput(window);

        // 第一步：渲染三角形到帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);

        // 清除为白色背景
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 渲染蓝色三角形
        glUseProgram(sceneShader);
        glBindVertexArray(triangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 第二步：使用帧缓冲纹理渲染到默认帧缓冲，应用模糊效果
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        // 清除屏幕
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 渲染屏幕四边形并应用模糊
        glUseProgram(screenShader);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 交换缓冲区并轮询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &triangleVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &triangleVBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteProgram(sceneShader);
    glDeleteProgram(screenShader);

    // 终止GLFW
    glfwTerminate();
    return 0;
}

// 处理输入
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 窗口大小改变回调
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
