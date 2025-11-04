#ifndef GLB_LOADER_H
#define GLB_LOADER_H

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION

#include "tiny_gltf.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <vector>

struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int indexCount;
    GLenum mode;
    GLenum indexType; // 添加索引类型
    std::vector<GLuint> textures;
};

class GLBLoader
{
  public:
    tinygltf::Model model;
    std::vector<Mesh> meshes;
    std::map<int, GLuint> textureMap;

    bool loadGLB(const std::string &filename)
    {
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);

        if (!warn.empty())
        {
            std::cout << "Warning: " << warn << std::endl;
        }

        if (!err.empty())
        {
            std::cerr << "Error: " << err << std::endl;
        }

        if (!ret)
        {
            std::cerr << "Failed to load glTF: " << filename << std::endl;
            return false;
        }

        // 加载纹理
        loadTextures();

        // 加载网格
        loadMeshes();

        return true;
    }

    void loadTextures()
    {
        for (size_t i = 0; i < model.textures.size(); i++)
        {
            tinygltf::Texture &tex = model.textures[i];
            if (tex.source < 0 || tex.source >= model.images.size())
                continue;

            tinygltf::Image &image = model.images[tex.source];

            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            GLenum format = GL_RGBA;
            if (image.component == 3)
                format = GL_RGB;
            else if (image.component == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE,
                         &image.image[0]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            textureMap[i] = textureID;
        }
    }

    void loadMeshes()
    {
        for (const auto &mesh : model.meshes)
        {
            for (const auto &primitive : mesh.primitives)
            {
                Mesh glMesh;

                // 创建 VAO
                glGenVertexArrays(1, &glMesh.vao);
                glBindVertexArray(glMesh.vao);

                // 加载顶点数据
                const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
                const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
                const tinygltf::Buffer &posBuffer = model.buffers[posView.buffer];

                glGenBuffers(1, &glMesh.vbo);
                glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);
                glBufferData(GL_ARRAY_BUFFER, posView.byteLength, &posBuffer.data[posView.byteOffset], GL_STATIC_DRAW);

                // 位置属性
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
                glEnableVertexAttribArray(0);

                // 加载纹理坐标（如果有）
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    const tinygltf::Accessor &texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                    const tinygltf::BufferView &texView = model.bufferViews[texAccessor.bufferView];
                    const tinygltf::Buffer &texBuffer = model.buffers[texView.buffer];

                    GLuint texVBO;
                    glGenBuffers(1, &texVBO);
                    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
                    glBufferData(GL_ARRAY_BUFFER, texView.byteLength, &texBuffer.data[texView.byteOffset],
                                 GL_STATIC_DRAW);

                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
                    glEnableVertexAttribArray(1);
                }

                // 加载索引
                if (primitive.indices >= 0)
                {
                    const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView &indexView = model.bufferViews[indexAccessor.bufferView];
                    const tinygltf::Buffer &indexBuffer = model.buffers[indexView.buffer];

                    glGenBuffers(1, &glMesh.ebo);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ebo);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexView.byteLength, &indexBuffer.data[indexView.byteOffset],
                                 GL_STATIC_DRAW);

                    glMesh.indexCount = indexAccessor.count;

                    // 根据 accessor 的 componentType 设置索引类型
                    switch (indexAccessor.componentType)
                    {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        glMesh.indexType = GL_UNSIGNED_INT;
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        glMesh.indexType = GL_UNSIGNED_SHORT;
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        glMesh.indexType = GL_UNSIGNED_BYTE;
                        break;
                    default:
                        glMesh.indexType = GL_UNSIGNED_SHORT;
                        break;
                    }
                }
                else
                {
                    glMesh.ebo = 0;
                    glMesh.indexCount = posAccessor.count;
                    glMesh.indexType = GL_UNSIGNED_SHORT;
                }

                glMesh.mode = primitive.mode;

                // 加载纹理
                if (primitive.material >= 0)
                {
                    const tinygltf::Material &mat = model.materials[primitive.material];
                    if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
                    {
                        int texIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
                        if (textureMap.find(texIndex) != textureMap.end())
                        {
                            glMesh.textures.push_back(textureMap[texIndex]);
                        }
                    }
                }

                glBindVertexArray(0);
                meshes.push_back(glMesh);
            }
        }
    }

    void render(GLuint shaderProgram)
    {
        for (const auto &mesh : meshes)
        {
            glBindVertexArray(mesh.vao);

            // 绑定纹理
            if (mesh.textures.size() > 0)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.textures[0]);
                glUniform1i(glGetUniformLocation(shaderProgram, "texture0"), 0);
            }

            if (mesh.ebo != 0)
            {
                glDrawElements(mesh.mode, mesh.indexCount, mesh.indexType, 0);
            }
            else
            {
                glDrawArrays(mesh.mode, 0, mesh.indexCount);
            }

            glBindVertexArray(0);
        }
    }

    void cleanup()
    {
        for (auto &mesh : meshes)
        {
            glDeleteVertexArrays(1, &mesh.vao);
            glDeleteBuffers(1, &mesh.vbo);
            if (mesh.ebo != 0)
            {
                glDeleteBuffers(1, &mesh.ebo);
            }
        }
        for (auto &tex : textureMap)
        {
            glDeleteTextures(1, &tex.second);
        }
    }

    ~GLBLoader()
    {
        cleanup();
    }
};

#endif // GLB_LOADER_H
