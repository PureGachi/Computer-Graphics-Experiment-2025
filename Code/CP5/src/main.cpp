#include <cmath>
#include <iostream>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// GLM 数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 其他头文件
#include "Camera.h"
#include "Light.h"
#include "Utils.h"

// 函数声明
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void do_movement();

// 窗口尺寸
const GLuint WIDTH = 1600, HEIGHT = 1200;

// 相机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool mouseCapture = true; // 鼠标捕获状态
bool firstMouse = true;

// 光源属性
PointLight light(glm::vec3(1.2f, 1.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f));
bool lightAnimation = true;  // 光源动画开关
float lightIntensity = 1.0f; // 光照强度

// 时间差
GLfloat deltaTime = 0.0f; // 当前帧与上一帧的时间差
GLfloat lastFrame = 0.0f; // 上一帧的时间

// 主函数，程序入口和游戏循环
int main()
{
    // 初始化GLFW
    glfwInit();
    // 设置GLFW的所有必要选项
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // 创建一个GLFW窗口对象，我们可以用它来调用GLFW的函数
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "光照效果", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // 设置所需的回调函数
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW 选项
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化GLAD以设置OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD初始化失败" << std::endl;
        return -1;
    }

    // 设置 Dear ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘控制

    // 设置 Dear ImGui 样式
    ImGui::StyleColorsDark();

    // 设置平台/渲染器后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // 定义视口维度
    glViewport(0, 0, WIDTH, HEIGHT);

    // OpenGL 选项
    glEnable(GL_DEPTH_TEST);

    // 构建并编译我们的着色器程序
    GLuint shaderProgram = Utils::createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint lightShaderProgram = Utils::createShaderProgram("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

    // 设置顶点数据（和缓冲区）和属性指针
    GLfloat vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
                          0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
                          -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

                          -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,
                          0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                          -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

                          -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,
                          -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
                          -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

                          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,
                          0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
                          0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

                          -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
                          0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
                          -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

                          -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
                          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                          -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};
    // 首先，设置容器的VAO（和VBO）
    GLuint VBO, containerVAO;
    glGenVertexArrays(1, &containerVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(containerVAO);
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    // 法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    //渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算当前帧的时间差
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 检查是否有任何事件被激活（按键、鼠标移动等）并调用相应的响应函数
        glfwPollEvents();
        do_movement();

        // 开始 Dear ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui 灯光控制面板
        ImGui::Begin("Light Control Panel");

        // 光照强度滑条
        ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 3.0f);

        // 分组：光照分量强度
        ImGui::Separator();
        ImGui::Text("Light Components:");
        ImGui::SliderFloat("Ambient Strength", &light.ambient, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse Strength", &light.diffuse, 0.0f, 2.0f);
        ImGui::SliderFloat("Specular Strength", &light.specular, 0.0f, 2.0f);
        ImGui::Separator();

        // 光源位置滑条
        float lightPos[3] = {light.getPosition().x, light.getPosition().y, light.getPosition().z};
        if (ImGui::SliderFloat3("Light Position", lightPos, -5.0f, 5.0f))
        {
            light.setPosition(glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
            lightAnimation = false; // 手动调整时停止动画
        }

        // 光源颜色滑条
        float lightCol[3] = {light.getColor().x, light.getColor().y, light.getColor().z};
        if (ImGui::ColorEdit3("Light Color", lightCol))
        {
            light.setColor(glm::vec3(lightCol[0], lightCol[1], lightCol[2]));
        }

        // 动画开关
        ImGui::Checkbox("Light Animation", &lightAnimation);

        // 鼠标捕获状态显示
        ImGui::Text("Mouse Capture: %s (Press TAB to toggle)", mouseCapture ? "ON" : "OFF");

        // 显示FPS
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::End();

        // 更新光源位置（仅当动画启用时）
        if (lightAnimation)
        {
            float time = static_cast<float>(glfwGetTime());
            light.updatePosition(time);
        }

        // 清除颜色缓冲区
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 在设置uniform或绘制对象时使用着色器程序
        glUseProgram(shaderProgram);

        // 设置光照uniform
        GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
        glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

        // 使用PointLight类设置光源uniform
        light.setSimpleUniforms(shaderProgram, lightIntensity);

        // 创建相机变换
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection =
            glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

        // 获取uniform位置
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        // 将矩阵传递给着色器
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 绘制容器（使用容器的顶点属性）
        glBindVertexArray(containerVAO);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // 同时绘制灯对象（光源立方体）
        glUseProgram(lightShaderProgram);
        // 获取灯光着色器上矩阵的位置对象
        GLint lightModelLoc = glGetUniformLocation(lightShaderProgram, "model");
        GLint lightViewLoc = glGetUniformLocation(lightShaderProgram, "view");
        GLint lightProjLoc = glGetUniformLocation(lightShaderProgram, "projection");

        // 为灯光立方体设置矩阵
        glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

        model = glm::mat4(1.0f);
        model = glm::translate(model, light.getPosition());
        model = glm::scale(model, glm::vec3(0.2f)); // 使其成为一个更小的立方体
        glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // 绘制灯光对象（使用灯光的顶点属性）
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // 渲染ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 交换屏幕缓冲区
        glfwSwapBuffers(window);
    }

    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 终止GLFW，清除GLFW分配的所有资源。
    glfwTerminate();
    return 0;
}

// 每当通过GLFW按下/释放键时调用
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Tab键切换鼠标捕获模式
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        mouseCapture = !mouseCapture;
        if (mouseCapture)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // 重置first mouse标志
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        std::cout << "Mouse capture: " << (mouseCapture ? "ON" : "OFF") << std::endl;
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement()
{
    // 相机控制
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    // 只有在鼠标捕获模式下才处理相机控制
    if (!mouseCapture)
        return;

    if (firstMouse)
    {
        lastX = static_cast<GLfloat>(xpos);
        lastY = static_cast<GLfloat>(ypos);
        firstMouse = false;
    }

    GLfloat xoffset = static_cast<GLfloat>(xpos - lastX);
    GLfloat yoffset = static_cast<GLfloat>(lastY - ypos); // Y坐标从下到上，所以反转

    lastX = static_cast<GLfloat>(xpos);
    lastY = static_cast<GLfloat>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // 只有在鼠标捕获模式下才处理缩放
    if (mouseCapture)
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}