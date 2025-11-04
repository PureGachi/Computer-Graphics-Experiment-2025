#include <glad/glad.h>
//需要注意glad需要在GLFW之前引入
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <string>

#include "Camera.h"
#include "GLBLoader.h"
#include "Utils.h"

// 相机对象
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

// 鼠标参数
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool mouseControlEnabled = true; // 是否启用鼠标控制

// 时间参数
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 窗口大小
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// 回调函数声明
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "相机实验", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "GLFW窗口创建失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 不再默认捕获鼠标，让用户可以操作 ImGui
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD初始化失败" << std::endl;
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // 天空盒需要 LEQUAL

    // 禁用背面剔除，因为我们在球体内部观察
    glDisable(GL_CULL_FACE);

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 编译着色器程序
    unsigned int shaderProgram = Utils::createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // 加载天空盒模型
    GLBLoader skyboxLoader;
    if (!skyboxLoader.loadGLB("resources/skybox.glb"))
    {
        std::cerr << "天空盒模型加载失败" << std::endl;
        return -1;
    }

    std::cout << "========== 天空盒渲染器 ==========" << std::endl;
    std::cout << "控制说明:" << std::endl;
    std::cout << "  在 ImGui 窗口中切换相机模式" << std::endl;
    std::cout << "  欧拉角模式 - 使用 Yaw/Pitch 角度" << std::endl;
    std::cout << "  四元数模式 - 使用 Yaw/Pitch/Roll 角度" << std::endl;
    std::cout << "  WASD     - 移动相机" << std::endl;
    std::cout << "  滚轮     - 缩放视野" << std::endl;
    std::cout << "  R        - 重置相机" << std::endl;
    std::cout << "  ESC      - 退出程序" << std::endl;
    std::cout << "=====================================" << std::endl;

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算帧时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入处理
        processInput(window);

        // 启动 ImGui 新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 创建 ImGui 控制窗口
        ImGui::Begin("Camera Controls");

        // 相机模式切换
        static int currentMode = 0; // 0 = Euler, 1 = Quaternion
        const char *modes[] = {"Euler Angles", "Quaternion"};

        ImGui::Text("Camera Mode:");
        if (ImGui::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
        {
            camera.SetMode(currentMode == 0 ? EULER : QUATERNION);
        }

        ImGui::Separator();

        // 获取当前的 Yaw 和 Pitch
        static float yaw = camera.Yaw;
        static float pitch = camera.Pitch;
        static float roll = 0.0f; // Roll 角度（仅四元数模式使用）

        // 增量旋转控制
        static float deltaYaw = 0.0f;
        static float deltaPitch = 0.0f;
        static float deltaRoll = 0.0f;

        if (currentMode == 0) // 欧拉角模式
        {
            ImGui::Text("Euler Angles Mode (3 DOF with Roll):");
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Warning: Gimbal lock at Pitch = +/-90 degrees!");
            ImGui::Spacing();

            // 显示当前角度
            ImGui::Text("Current Angles:");
            ImGui::Text("Yaw: %.2f degrees", camera.Yaw);
            ImGui::Text("Pitch: %.2f degrees", camera.Pitch);
            ImGui::Text("Roll: %.2f degrees", camera.Roll);
            ImGui::Separator();

            // 增量旋转控制（这里会出现万向锁）
            ImGui::Text("Incremental Rotation (Apply Step by Step):");

            ImGui::SliderFloat("Delta Yaw", &deltaYaw, -10.0f, 10.0f);
            ImGui::SliderFloat("Delta Pitch", &deltaPitch, -10.0f, 10.0f);
            ImGui::SliderFloat("Delta Roll", &deltaRoll, -10.0f, 10.0f);

            if (ImGui::Button("Apply Rotation", ImVec2(200, 30)))
            {
                yaw = camera.Yaw + deltaYaw;
                pitch = camera.Pitch + deltaPitch;
                roll = camera.Roll + deltaRoll;
                camera.SetEulerAngles(yaw, pitch, roll, false);
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset Deltas"))
            {
                deltaYaw = 0.0f;
                deltaPitch = 0.0f;
                deltaRoll = 0.0f;
            }

            ImGui::Separator();
            ImGui::Text("Direct Control:");

            // 滑动条控制 Yaw（偏航角：-180 到 180）
            if (ImGui::SliderFloat("Yaw (Horizontal)", &yaw, -180.0f, 180.0f))
            {
                camera.SetEulerAngles(yaw, camera.Pitch, camera.Roll, false);
            }

            // 滑动条控制 Pitch（俯仰角：无限制）
            if (ImGui::SliderFloat("Pitch (Vertical)", &pitch, -180.0f, 180.0f))
            {
                camera.SetEulerAngles(camera.Yaw, pitch, camera.Roll, false);
            }

            // 滑动条控制 Roll（翻滚角）
            if (ImGui::SliderFloat("Roll (Tilt)", &roll, -180.0f, 180.0f))
            {
                camera.SetEulerAngles(camera.Yaw, camera.Pitch, roll, false);
            }

            ImGui::Separator();

            // 输入框精确控制
            ImGui::Text("Precise Input:");
            if (ImGui::InputFloat("Yaw Input", &yaw, 1.0f, 10.0f, "%.2f"))
            {
                camera.SetEulerAngles(yaw, camera.Pitch, camera.Roll, false);
            }
            if (ImGui::InputFloat("Pitch Input", &pitch, 1.0f, 10.0f, "%.2f"))
            {
                camera.SetEulerAngles(camera.Yaw, pitch, camera.Roll, false);
            }
            if (ImGui::InputFloat("Roll Input", &roll, 1.0f, 10.0f, "%.2f"))
            {
                camera.SetEulerAngles(camera.Yaw, camera.Pitch, roll, false);
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                               "Try: Set Pitch to 90, then apply Delta Yaw.\nYou'll see gimbal lock!");
        }
        else // 四元数模式
        {
            ImGui::Text("Quaternion Mode:");
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "No gimbal lock with proper quaternion operations!");
            ImGui::Spacing();

            // 显示当前四元数
            ImGui::Text("Current Quaternion:");
            ImGui::Text("x: %.3f, y: %.3f", camera.Orientation.x, camera.Orientation.y);
            ImGui::Text("z: %.3f, w: %.3f", camera.Orientation.z, camera.Orientation.w);
            ImGui::Separator();

            // 增量旋转控制（使用四元数，不会有万向锁）
            ImGui::Text("Incremental Rotation (Apply Step by Step):");
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Using local axes - no gimbal lock!");

            ImGui::SliderFloat("Delta Yaw (Local Up)", &deltaYaw, -10.0f, 10.0f);
            ImGui::SliderFloat("Delta Pitch (Local Right)", &deltaPitch, -10.0f, 10.0f);
            ImGui::SliderFloat("Delta Roll (Local Front)", &deltaRoll, -10.0f, 10.0f);

            if (ImGui::Button("Apply Rotation", ImVec2(200, 30)))
            {
                // 使用局部轴进行增量旋转
                glm::quat qYaw = glm::angleAxis(glm::radians(deltaYaw), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::quat qPitch = glm::angleAxis(glm::radians(deltaPitch), camera.Right);
                glm::quat qRoll = glm::angleAxis(glm::radians(deltaRoll), camera.Front);

                // 应用增量旋转：先世界Yaw，再局部Pitch和Roll
                camera.Orientation = qYaw * camera.Orientation * qPitch * qRoll;
                camera.Orientation = glm::normalize(camera.Orientation);
                camera.ProcessMouseMovement(0, 0); // 触发更新向量
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset Deltas"))
            {
                deltaYaw = 0.0f;
                deltaPitch = 0.0f;
                deltaRoll = 0.0f;
            }

            ImGui::Separator();
            ImGui::Text("Direct Control (for comparison):");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                               "Note: This uses Euler->Quaternion conversion\nand still has gimbal lock!");

            bool quatChanged = false;

            if (ImGui::SliderFloat("Yaw (Y-axis)", &yaw, -180.0f, 180.0f))
            {
                quatChanged = true;
            }

            if (ImGui::SliderFloat("Pitch (X-axis)", &pitch, -180.0f, 180.0f))
            {
                quatChanged = true;
            }

            if (ImGui::SliderFloat("Roll (Z-axis)", &roll, -180.0f, 180.0f))
            {
                quatChanged = true;
            }

            // 应用四元数旋转
            if (quatChanged)
            {
                camera.SetQuaternionEulerAngles(yaw, pitch, roll);
            }

            ImGui::Separator();
            ImGui::Text("Precise Input:");
            if (ImGui::InputFloat("Yaw Input", &yaw, 1.0f, 10.0f, "%.2f"))
            {
                camera.SetQuaternionEulerAngles(yaw, pitch, roll);
            }
            if (ImGui::InputFloat("Pitch Input", &pitch, 1.0f, 10.0f, "%.2f"))
            {
                camera.SetQuaternionEulerAngles(yaw, pitch, roll);
            }
            if (ImGui::InputFloat("Roll Input", &roll, 1.0f, 10.0f, "%.2f"))
            {
                camera.SetQuaternionEulerAngles(yaw, pitch, roll);
            }

            ImGui::Separator();
            ImGui::TextColored(
                ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                "Comparison: Try setting Pitch=90 in Direct Control,\nthen switch to Incremental - still works!");
        }

        ImGui::Separator();

        if (ImGui::Button("Reset Camera"))
        {
            camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f));
            camera.SetMode(currentMode == 0 ? EULER : QUATERNION);
            yaw = camera.Yaw;
            pitch = camera.Pitch;
            roll = 0.0f;
            deltaYaw = 0.0f;
            deltaPitch = 0.0f;
            deltaRoll = 0.0f;
        }

        ImGui::End();

        // 渲染
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 设置变换矩阵
        glm::mat4 model = glm::mat4(1.0f);
        // 绕 X 轴旋转 -90 度（反向旋转），将球体"立起来"，让初始视角看到正确的上半部分
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection =
            glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // 传递矩阵给着色器
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 渲染天空盒
        skyboxLoader.render(shaderProgram);

        // 渲染 ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 交换缓冲区和轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理 ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 清理资源
    glDeleteProgram(shaderProgram);
    skyboxLoader.cleanup();

    glfwTerminate();
    return 0;
}

// 处理输入
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 相机移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // 重置相机：按R键
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed)
    {
        rKeyPressed = true;
        camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f));
        std::cout << "相机已重置" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    {
        rKeyPressed = false;
    }
}

// 鼠标移动回调（已禁用，使用 ImGui 滑动条控制）
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    // 不使用鼠标控制相机
}

// 滚轮回调
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 窗口大小改变回调
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
