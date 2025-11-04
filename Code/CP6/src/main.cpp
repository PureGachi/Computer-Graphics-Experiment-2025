#include <cmath>
#include <iostream>

// OpenGL加载器
#include <glad/glad.h>

// 窗口和输入处理
#include <GLFW/glfw3.h>

// 图形用户界面
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// 数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 其他头文件
#include "Camera.h"
#include "Light.h"
#include "OBJLoader.h"
#include "Utils.h"

// 函数声明
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void do_movement();

// 窗口尺寸
GLuint WIDTH = 1600, HEIGHT = 1200;

// 相机
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool mouseCapture = true; // 鼠标捕获状态
bool firstMouse = true;

// 光源属性 - PBR需要多个光源
glm::vec3 lightPositions[] = {glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(10.0f, 10.0f, 10.0f),
                              glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(10.0f, -10.0f, 10.0f)};
glm::vec3 lightColors[] = {glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(300.0f, 300.0f, 300.0f),
                           glm::vec3(300.0f, 300.0f, 300.0f), glm::vec3(300.0f, 300.0f, 300.0f)};

bool lightAnimation = false; // 光源动画开关

// 模型属性
float modelScale = 1.0f;
glm::vec3 modelRotation(0.0f);

// PBR材质参数
Utils::PBRMaterial pbrMaterial;

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

    // 创建GLFW窗口对象
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "基于PBR渲染的模型查看器", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // 设置必要的回调函数
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLFW选项设置
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化GLAD以设置OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置Dear ImGui上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘控制

    // 设置Dear ImGui样式
    ImGui::StyleColorsDark();

    // 设置平台/渲染器后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // 定义视口尺寸
    glViewport(0, 0, WIDTH, HEIGHT);

    // OpenGL选项
    glEnable(GL_DEPTH_TEST);

    // 构建和编译着色器程序
    GLuint shaderProgram = Utils::createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint lightShaderProgram = Utils::createShaderProgram("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

    // 自动搜索并加载模型文件
    std::string modelDirectory = "resources/batmanCar";
    Utils::ModelFiles modelFiles = Utils::findModelFiles(modelDirectory);

    if (!modelFiles.valid)
    {
        std::cerr << "模型文件搜索失败!" << std::endl;
        return -1;
    }

    // 加载OBJ模型
    OBJLoader carModel;
    if (!carModel.loadOBJ(modelFiles.objPath))
    {
        std::cerr << "Failed to load car model: " << modelFiles.objPath << std::endl;
        return -1;
    }

    // 从MTL文件加载PBR材质
    std::map<std::string, Utils::PBRMaterial> materials;
    if (!modelFiles.mtlPath.empty())
    {
        materials = Utils::loadMTL(modelFiles.mtlPath, modelFiles.directory);
    }

    // 使用第一个材质或创建默认材质
    if (!materials.empty())
    {
        pbrMaterial = materials.begin()->second;
        std::cout << "Using material: " << materials.begin()->first << std::endl;
    }
    else
    {
        pbrMaterial = Utils::createDefaultPBRMaterial();
        std::cout << "Using default PBR material" << std::endl;
    }

    // 光源立方体顶点（用于可视化）
    GLfloat lightVertices[] = {
        // 背面
        -0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        // 正面
        -0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        // 左面
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        // 右面
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        // 底面
        -0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        0.1f,
        -0.1f,
        -0.1f,
        -0.1f,
        // 顶面
        -0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        0.1f,
        -0.1f,
        0.1f,
        -0.1f,
    };

    GLuint lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // 游戏循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算当前帧的时间差
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 检查是否有事件被激活（按键、鼠标移动等）并调用相应的响应函数
        glfwPollEvents();
        do_movement();

        // 开始Dear ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui控制面板
        ImGui::Begin("Car Model Control Panel");

        // 光照控制
        ImGui::Text("Lighting:");
        static int selectedLight = 0;
        ImGui::SliderInt("Light Index", &selectedLight, 0, 3);

        float lightPos[3] = {lightPositions[selectedLight].x, lightPositions[selectedLight].y,
                             lightPositions[selectedLight].z};
        if (ImGui::SliderFloat3("Light Position", lightPos, -20.0f, 20.0f))
        {
            lightPositions[selectedLight] = glm::vec3(lightPos[0], lightPos[1], lightPos[2]);
        }

        float lightCol[3] = {lightColors[selectedLight].x / 300.0f, lightColors[selectedLight].y / 300.0f,
                             lightColors[selectedLight].z / 300.0f};
        if (ImGui::ColorEdit3("Light Color", lightCol))
        {
            lightColors[selectedLight] = glm::vec3(lightCol[0], lightCol[1], lightCol[2]) * 300.0f;
        }

        // 动画开关
        ImGui::Checkbox("Light Animation", &lightAnimation);
        ImGui::Separator();

        // 模型变换控制
        ImGui::Text("Model Transform:");
        ImGui::SliderFloat("Scale", &modelScale, 0.1f, 3.0f);
        ImGui::SliderFloat3("Rotation", &modelRotation[0], -180.0f, 180.0f);
        ImGui::Separator();

        // 相机信息显示
        ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.Position.x, camera.Position.y, camera.Position.z);

        // 鼠标捕获状态显示
        ImGui::Text("Mouse Capture: %s (Press TAB to toggle)", mouseCapture ? "ON" : "OFF");

        // 显示FPS和模型信息
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Vertices: %d", (int)carModel.vertices.size());
        ImGui::Text("Triangles: %d", (int)carModel.indices.size() / 3);

        ImGui::End();

        // 更新光源位置（仅在动画启用时）
        if (lightAnimation)
        {
            float time = static_cast<float>(glfwGetTime());
            lightPositions[0] = glm::vec3(sin(time * 2.0f) * 10.0f, 10.0f, cos(time * 2.0f) * 10.0f);
        }

        // 清除颜色缓冲区
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器程序来设置uniform/绘制对象
        glUseProgram(shaderProgram);

        // 设置相机位置
        GLint camPosLoc = glGetUniformLocation(shaderProgram, "camPos");
        glUniform3f(camPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

        // 设置光源位置和颜色
        for (int i = 0; i < 4; ++i)
        {
            std::string lightPosName = "lightPositions[" + std::to_string(i) + "]";
            std::string lightColorName = "lightColors[" + std::to_string(i) + "]";

            GLint lightPosLoc = glGetUniformLocation(shaderProgram, lightPosName.c_str());
            GLint lightColorLoc = glGetUniformLocation(shaderProgram, lightColorName.c_str());

            glUniform3f(lightPosLoc, lightPositions[i].x, lightPositions[i].y, lightPositions[i].z);
            glUniform3f(lightColorLoc, lightColors[i].x, lightColors[i].y, lightColors[i].z);
        }

        // 设置PBR材质属性
        GLint albedoLoc = glGetUniformLocation(shaderProgram, "albedo");
        GLint metallicLoc = glGetUniformLocation(shaderProgram, "metallic");
        GLint roughnessLoc = glGetUniformLocation(shaderProgram, "roughness");
        GLint aoLoc = glGetUniformLocation(shaderProgram, "ao");

        glUniform3f(albedoLoc, pbrMaterial.albedo.x, pbrMaterial.albedo.y, pbrMaterial.albedo.z);
        glUniform1f(metallicLoc, pbrMaterial.metallic);
        glUniform1f(roughnessLoc, pbrMaterial.roughness);
        glUniform1f(aoLoc, pbrMaterial.ao);

        // 绑定PBR纹理
        int textureUnit = 0;

        // 反射率贴图
        if (pbrMaterial.albedoMap != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, pbrMaterial.albedoMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "albedoMap"), textureUnit);
            glUniform1i(glGetUniformLocation(shaderProgram, "useAlbedoMap"), 1);
            textureUnit++;
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgram, "useAlbedoMap"), 0);
        }

        // 金属度贴图
        if (pbrMaterial.metallicMap != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, pbrMaterial.metallicMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "metallicMap"), textureUnit);
            glUniform1i(glGetUniformLocation(shaderProgram, "useMetallicMap"), 1);
            textureUnit++;
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgram, "useMetallicMap"), 0);
        }

        // 粗糙度贴图
        if (pbrMaterial.roughnessMap != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, pbrMaterial.roughnessMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "roughnessMap"), textureUnit);
            glUniform1i(glGetUniformLocation(shaderProgram, "useRoughnessMap"), 1);
            textureUnit++;
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgram, "useRoughnessMap"), 0);
        }

        // 法线贴图
        if (pbrMaterial.normalMap != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, pbrMaterial.normalMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), textureUnit);
            glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMap"), 1);
            textureUnit++;
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMap"), 0);
        }

        // 环境遮蔽贴图
        if (pbrMaterial.aoMap != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, pbrMaterial.aoMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "aoMap"), textureUnit);
            glUniform1i(glGetUniformLocation(shaderProgram, "useAOMap"), 1);
            textureUnit++;
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgram, "useAOMap"), 0);
        }

        // 创建相机变换矩阵
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

        // 绘制汽车模型
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(modelScale));
        model = glm::rotate(model, glm::radians(modelRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(modelRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(modelRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        carModel.draw();

        // 同时绘制灯光对象（光源立方体）
        glUseProgram(lightShaderProgram);
        // 获取光源着色器中矩阵的位置对象
        GLint lightModelLoc = glGetUniformLocation(lightShaderProgram, "model");
        GLint lightViewLoc = glGetUniformLocation(lightShaderProgram, "view");
        GLint lightProjLoc = glGetUniformLocation(lightShaderProgram, "projection");

        // 为光源立方体设置矩阵
        glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(lightProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 绘制光源立方体用于可视化
        for (int i = 0; i < 4; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(1.5f)); // 使光源更大
            glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // 绘制光源对象
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

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

    // 清理资源
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &lightVBO);

    // 终止GLFW，清理GLFW分配的资源
    glfwTerminate();
    return 0;
}

// 当通过GLFW按下/释放按键时调用
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
            firstMouse = true; // 重置首次鼠标标志
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
    // 相机控制（仅在鼠标捕获启用时）
    if (mouseCapture)
    {
        if (keys[GLFW_KEY_W])
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keys[GLFW_KEY_S])
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keys[GLFW_KEY_A])
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (keys[GLFW_KEY_D])
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
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
    GLfloat yoffset = static_cast<GLfloat>(lastY - ypos); // 反转因为y坐标从下往上

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

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}
