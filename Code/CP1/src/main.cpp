#include <glad/glad.h>
//需要注意glad需要在GLFW之前引入
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// 窗口尺寸
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// 函数声明
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
std::string getExecutableDir();
std::string loadShaderSource(const std::string &filepath);
unsigned int compileShader(GLenum type, const std::string &source);
unsigned int createShaderProgram(const std::string &vertexPath, const std::string &fragmentPath);

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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HelloTriangle", NULL, NULL);
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

    // 创建着色器程序
    unsigned int shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (shaderProgram == 0)
    {
        std::cerr << "着色器程序创建失败" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 三角形顶点数据（位置和颜色）
    float vertices[] = {
        // 位置              // 颜色
        0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // 顶部 - 红色
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // 左下 - 绿色
        0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // 右下 - 蓝色
    };

    // 创建VAO和VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定VAO
    glBindVertexArray(VAO);

    // 绑定VBO并传输数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    std::cout << "Triangle Renderer Started!" << std::endl;
    std::cout << "Press ESC to exit" << std::endl;

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 输入处理
        processInput(window);

        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);
        // 绘制三角形
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 交换缓冲并轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

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

// 获取可执行文件所在目录
std::string getExecutableDir()
{
    return std::filesystem::current_path().string();
}

// 加载着色器源代码
std::string loadShaderSource(const std::string &filepath)
{
    // 尝试直接打开
    std::ifstream file(filepath);

    // 如果失败，尝试从可执行文件目录打开
    if (!file.is_open())
    {
        std::string exeDir = getExecutableDir();
        std::string fullPath = exeDir + "/" + filepath;
        file.open(fullPath);

        if (!file.is_open())
        {
            std::cerr << "错误：着色器文件读取失败: " << filepath << std::endl;
            std::cerr << "  当前目录: " << exeDir << std::endl;
            std::cerr << "  尝试路径: " << fullPath << std::endl;
            return "";
        }
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::cout << "着色器加载成功: " << filepath << std::endl;
    return buffer.str();
}

// 编译着色器
unsigned int compileShader(GLenum type, const std::string &source)
{
    unsigned int shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::string shaderType = (type == GL_VERTEX_SHADER) ? "顶点着色器" : "片段着色器";
        std::cerr << "错误：" << shaderType << "编译失败\n" << infoLog << std::endl;
        return 0;
    }

    return shader;
}

// 创建着色器程序
unsigned int createShaderProgram(const std::string &vertexPath, const std::string &fragmentPath)
{
    // 加载着色器源代码
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty())
    {
        return 0;
    }

    // 编译着色器
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        if (vertexShader)
            glDeleteShader(vertexShader);
        if (fragmentShader)
            glDeleteShader(fragmentShader);
        return 0;
    }

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 检查链接错误
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "错误：着色器程序链接失败\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // 删除着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
