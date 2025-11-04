#ifndef LIGHT_H
#define LIGHT_H

#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>

// 点光源类
class PointLight
{
  public:
    // 光源属性
    glm::vec3 position;
    glm::vec3 color;

    // 光照强度参数
    float ambient;
    float diffuse;
    float specular;

    // 构造函数
    PointLight(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 col = glm::vec3(1.0f, 1.0f, 1.0f))
        : position(pos), color(col), ambient(0.1f), diffuse(1.0f), specular(0.5f)
    {
    }

    // 更新光源位置（简单动画）
    void updatePosition(float time)
    {
        position.x = 1.0f + sin(time) * 2.0f;
        position.y = sin(time / 2.0f) * 1.0f;
        position.z = cos(time) * 2.0f;
    }

    // 设置光源位置
    void setPosition(const glm::vec3 &pos)
    {
        position = pos;
    }

    // 设置光源颜色
    void setColor(const glm::vec3 &col)
    {
        color = col;
    }

    // 获取位置
    const glm::vec3 &getPosition() const
    {
        return position;
    }

    // 获取颜色
    const glm::vec3 &getColor() const
    {
        return color;
    }

    // 设置光源参数到着色器（简化版本）
    void setSimpleUniforms(GLuint shaderProgram, float intensity = 1.0f) const
    {
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        GLint ambientLoc = glGetUniformLocation(shaderProgram, "ambientStrength");
        GLint diffuseLoc = glGetUniformLocation(shaderProgram, "diffuseStrength");
        GLint specularLoc = glGetUniformLocation(shaderProgram, "specularStrength");

        glUniform3f(lightPosLoc, position.x, position.y, position.z);
        // 应用强度到光源颜色
        glm::vec3 finalColor = color * intensity;
        glUniform3f(lightColorLoc, finalColor.x, finalColor.y, finalColor.z);

        // 设置光照强度参数
        glUniform1f(ambientLoc, ambient);
        glUniform1f(diffuseLoc, diffuse);
        glUniform1f(specularLoc, specular);
    }
};

#endif