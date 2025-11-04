#include "shader.hpp"

Shader::Shader(const char *vertexPath, const char *fragPath)
{

    std::string vertexCode, fragCode, geometryCode;
    std::ifstream vertexFile, fragFile, geometryFile;
    std::stringstream vertexStream, fragStream, geometryStream;

    vertexFile.open(vertexPath);
    fragFile.open(fragPath);

    if (!vertexFile.is_open() || !fragFile.is_open())
    {
        throw std::runtime_error("failed open Shader file");
    }

    // 将缓冲区内如读入对应的字符串流
    vertexStream << vertexFile.rdbuf();
    fragStream << fragFile.rdbuf();

    // 关闭文件
    vertexFile.close();
    fragFile.close();

    // 将流对象转换为string
    vertexCode = vertexStream.str();
    fragCode = fragStream.str();

    // 将string转换为code
    const char *vertShaderCode = vertexCode.c_str();
    const char *fragShaderCode = fragCode.c_str();

    // 创建着色器模块
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint geom = glCreateShader(GL_GEOMETRY_SHADER);

    glShaderSource(vertex, 1, &vertShaderCode, NULL);
    glShaderSource(frag, 1, &fragShaderCode, NULL);

    glCompileShader(vertex);
    glCompileShader(frag);

    // 检查是否编译成功
    int success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        throw std::runtime_error("failed complie vertexshader");
    }
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        throw std::runtime_error("failed complie fragshader");
    }
    // 创建着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, frag);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        throw std::runtime_error("failed to link program");
    }

    // 删除着色器模块
    glDeleteShader(vertex);
    glDeleteShader(frag);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string &name, glm::vec3 nums) const
{
    // 获取着色器内的uniform变量索引
    GLint loactionnums = glGetUniformLocation(ID, name.c_str());
    // 将索要设置的uniform值输入给着色器
    glUniform3f(loactionnums, nums.x,nums.y,nums.z);
}

void Shader::setMat4(const std::string &name, glm::mat4 mats) const
{
    // 获取着色器内的uniform变量索引
    unsigned int locationnums = glGetUniformLocation(ID, name.c_str());
    // 将索要设置的uniform值输入给着色器
    glUniformMatrix4fv(locationnums, 1, GL_FALSE, glm::value_ptr(mats));
}