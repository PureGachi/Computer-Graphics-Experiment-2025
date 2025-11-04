#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// 前置声明，避免包含glad.h
typedef unsigned int GLenum;
typedef unsigned int GLuint;

namespace Utils
{
// 读取文件内容
inline std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "文件打开失败: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 编译着色器
inline unsigned int compileShader(unsigned int type, const std::string &source)
{
    unsigned int shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "着色器编译失败:\n" << infoLog << std::endl;
    }

    return shader;
}

// 创建着色器程序
inline unsigned int createShaderProgram(const std::string &vertexPath, const std::string &fragmentPath)
{
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "着色器程序链接失败:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
} // namespace Utils

#endif
