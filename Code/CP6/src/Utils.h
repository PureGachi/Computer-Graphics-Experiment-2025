#ifndef UTILS_H
#define UTILS_H

#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb/stb_image.h"

namespace Utils
{
// PBR材质结构体
struct PBRMaterial
{
    glm::vec3 albedo = glm::vec3(0.8f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;

    // 纹理ID
    unsigned int albedoMap = 0;
    unsigned int metallicMap = 0;
    unsigned int roughnessMap = 0;
    unsigned int normalMap = 0;
    unsigned int aoMap = 0;

    // 纹理路径
    std::string albedoPath;
    std::string metallicPath;
    std::string roughnessPath;
    std::string normalPath;
    std::string aoPath;
};
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

    if (vertexCode.empty() || fragmentCode.empty())
    {
        std::cerr << "着色器文件读取失败!" << std::endl;
        return 0;
    }

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

// 加载纹理
inline unsigned int loadTexture(const std::string &path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // OpenGL纹理坐标系统
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "纹理加载成功: " << path << " (" << width << "x" << height << ", " << nrChannels << " channels)"
                  << std::endl;
    }
    else
    {
        std::cerr << "纹理加载失败: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 读取MTL材质文件
inline std::map<std::string, PBRMaterial> loadMTL(const std::string &mtlPath, const std::string &baseDir = "")
{
    std::map<std::string, PBRMaterial> materials;
    std::ifstream file(mtlPath);

    if (!file.is_open())
    {
        std::cerr << "MTL文件打开失败: " << mtlPath << std::endl;
        return materials;
    }

    std::string currentMaterial;
    PBRMaterial material;

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "newmtl")
        {
            // 如果之前有材质，保存它
            if (!currentMaterial.empty())
            {
                materials[currentMaterial] = material;
            }

            // 开始新材质
            iss >> currentMaterial;
            material = PBRMaterial(); // 重置材质
        }
        else if (prefix == "Kd")
        {
            // 漫反射颜色 -> albedo
            iss >> material.albedo.r >> material.albedo.g >> material.albedo.b;
        }
        else if (prefix == "map_Kd")
        {
            // 漫反射贴图 -> albedo map
            iss >> material.albedoPath;
            if (!baseDir.empty())
            {
                material.albedoPath = baseDir + "/" + material.albedoPath;
            }
        }
        else if (prefix == "map_Pm")
        {
            // 金属度贴图
            iss >> material.metallicPath;
            if (!baseDir.empty())
            {
                material.metallicPath = baseDir + "/" + material.metallicPath;
            }
        }
        else if (prefix == "map_Pr")
        {
            // 粗糙度贴图
            iss >> material.roughnessPath;
            if (!baseDir.empty())
            {
                material.roughnessPath = baseDir + "/" + material.roughnessPath;
            }
        }
        else if (prefix == "map_Bump" || prefix == "bump")
        {
            // 法线贴图
            std::string remaining = line.substr(line.find(prefix) + prefix.length());
            std::istringstream remainingStream(remaining);
            std::string temp;

            // 跳过可能的参数（如-bm 1.0）
            while (remainingStream >> temp)
            {
                // 如果包含文件扩展名，认为是文件名
                if (temp.find(".png") != std::string::npos || temp.find(".jpg") != std::string::npos ||
                    temp.find(".jpeg") != std::string::npos || temp.find(".tga") != std::string::npos ||
                    temp.find(".bmp") != std::string::npos)
                {
                    material.normalPath = temp;
                    break;
                }
            }
            if (!baseDir.empty() && !material.normalPath.empty())
            {
                material.normalPath = baseDir + "/" + material.normalPath;
            }
        }
        else if (prefix == "map_Ka")
        {
            // 环境光遮蔽贴图
            iss >> material.aoPath;
            if (!baseDir.empty())
            {
                material.aoPath = baseDir + "/" + material.aoPath;
            }
        }
        else if (prefix == "Ns")
        {
            // 镜面指数 -> 转换为粗糙度
            float ns;
            iss >> ns;
            // 将镜面指数转换为粗糙度（简单映射）
            material.roughness = 1.0f - (ns / 1000.0f);
            material.roughness = glm::clamp(material.roughness, 0.0f, 1.0f);
        }
    }

    // 保存最后一个材质
    if (!currentMaterial.empty())
    {
        materials[currentMaterial] = material;
    }

    file.close();

    // 加载所有纹理
    for (auto &[name, mat] : materials)
    {
        if (!mat.albedoPath.empty())
        {
            mat.albedoMap = loadTexture(mat.albedoPath);
        }
        if (!mat.metallicPath.empty())
        {
            mat.metallicMap = loadTexture(mat.metallicPath);
        }
        if (!mat.roughnessPath.empty())
        {
            mat.roughnessMap = loadTexture(mat.roughnessPath);
        }
        if (!mat.normalPath.empty())
        {
            mat.normalMap = loadTexture(mat.normalPath);
        }
        if (!mat.aoPath.empty())
        {
            mat.aoMap = loadTexture(mat.aoPath);
        }
    }

    std::cout << "MTL文件加载成功: " << materials.size() << " 个材质" << std::endl;
    return materials;
}

// 创建默认PBR材质
inline PBRMaterial createDefaultPBRMaterial()
{
    PBRMaterial material;
    material.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
    material.metallic = 0.0f;
    material.roughness = 0.5f;
    material.ao = 1.0f;
    return material;
}

// 在指定文件夹中搜索文件（按扩展名）
inline std::vector<std::string> findFilesInDirectory(const std::string &directory, const std::string &extension)
{
    std::vector<std::string> files;

    try
    {
        if (!std::filesystem::exists(directory))
        {
            std::cerr << "目录不存在: " << directory << std::endl;
            return files;
        }

        for (const auto &entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().string();
                if (filename.size() >= extension.size() &&
                    filename.compare(filename.size() - extension.size(), extension.size(), extension) == 0)
                {
                    files.push_back(filename);
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "文件系统错误: " << e.what() << std::endl;
    }

    return files;
}

// 模型文件信息结构体
struct ModelFiles
{
    std::string objPath;
    std::string mtlPath;
    std::string directory;
    bool valid = false;
};

// 自动在文件夹中查找OBJ和MTL文件
inline ModelFiles findModelFiles(const std::string &directory)
{
    ModelFiles result;
    result.directory = directory;

    // 搜索OBJ文件
    std::vector<std::string> objFiles = findFilesInDirectory(directory, ".obj");

    // 搜索MTL文件
    std::vector<std::string> mtlFiles = findFilesInDirectory(directory, ".mtl");

    if (objFiles.empty())
    {
        std::cerr << "在目录 " << directory << " 中未找到.obj文件" << std::endl;
        return result;
    }

    // 使用第一个找到的OBJ文件
    result.objPath = objFiles[0];
    std::cout << "找到OBJ文件: " << result.objPath << std::endl;

    if (objFiles.size() > 1)
    {
        std::cout << "注意: 找到多个OBJ文件，使用第一个: " << result.objPath << std::endl;
        for (size_t i = 1; i < objFiles.size(); ++i)
        {
            std::cout << "  其他文件: " << objFiles[i] << std::endl;
        }
    }

    // 查找对应的MTL文件
    if (!mtlFiles.empty())
    {
        result.mtlPath = mtlFiles[0];
        std::cout << "找到MTL文件: " << result.mtlPath << std::endl;

        if (mtlFiles.size() > 1)
        {
            std::cout << "注意: 找到多个MTL文件，使用第一个: " << result.mtlPath << std::endl;
        }
    }
    else
    {
        std::cout << "警告: 在目录 " << directory << " 中未找到.mtl文件" << std::endl;
    }

    result.valid = true;
    return result;
}

} // namespace Utils

#endif