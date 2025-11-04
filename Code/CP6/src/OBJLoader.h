#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class OBJLoader {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    
    bool loadOBJ(const std::string& path) {
        std::vector<glm::vec3> temp_vertices;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;
        std::vector<unsigned int> vertex_indices, uv_indices, normal_indices;
        
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open OBJ file: " << path << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            
            if (prefix == "v") {
                // 顶点位置
                glm::vec3 vertex;
                iss >> vertex.x >> vertex.y >> vertex.z;
                temp_vertices.push_back(vertex);
            }
            else if (prefix == "vt") {
                // 纹理坐标
                glm::vec2 uv;
                iss >> uv.x >> uv.y;
                temp_uvs.push_back(uv);
            }
            else if (prefix == "vn") {
                // 法线
                glm::vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                temp_normals.push_back(normal);
            }
            else if (prefix == "f") {
                // 面
                std::string vertex1, vertex2, vertex3;
                iss >> vertex1 >> vertex2 >> vertex3;
                
                parseVertex(vertex1, vertex_indices, uv_indices, normal_indices);
                parseVertex(vertex2, vertex_indices, uv_indices, normal_indices);
                parseVertex(vertex3, vertex_indices, uv_indices, normal_indices);
            }
        }
        
        file.close();
        
        // 如果没有法线，生成法线
        bool hasNormals = !temp_normals.empty() && normal_indices.size() == vertex_indices.size();
        if (!hasNormals) {
            std::cout << "No normals found in OBJ file, generating normals..." << std::endl;
            temp_normals = generateNormals(temp_vertices, vertex_indices);
            normal_indices = vertex_indices; // 使用相同的索引
        }
        
        // 构建最终的顶点数据
        buildVertexData(temp_vertices, temp_uvs, temp_normals, 
                       vertex_indices, uv_indices, normal_indices, hasNormals);
        
        setupMesh();
        
        std::cout << "OBJ loaded successfully: " << vertices.size() << " vertices, " 
                  << indices.size() / 3 << " triangles" << std::endl;
        return true;
    }
    
private:
    void parseVertex(const std::string& vertex, 
                    std::vector<unsigned int>& vertex_indices,
                    std::vector<unsigned int>& uv_indices,
                    std::vector<unsigned int>& normal_indices) {
        std::istringstream iss(vertex);
        std::string token;
        
        // 解析 "v/vt/vn" 格式
        if (std::getline(iss, token, '/')) {
            vertex_indices.push_back(std::stoi(token) - 1); // OBJ索引从1开始
        }
        if (std::getline(iss, token, '/')) {
            if (!token.empty()) {
                uv_indices.push_back(std::stoi(token) - 1);
            }
        }
        if (std::getline(iss, token)) {
            if (!token.empty()) {
                normal_indices.push_back(std::stoi(token) - 1);
            }
        }
    }
    
    std::vector<glm::vec3> generateNormals(const std::vector<glm::vec3>& vertices,
                                          const std::vector<unsigned int>& indices) {
        std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f));
        
        // 为每个三角形计算法线并累加到顶点
        for (size_t i = 0; i < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];
            
            glm::vec3 v0 = vertices[i0];
            glm::vec3 v1 = vertices[i1];
            glm::vec3 v2 = vertices[i2];
            
            // 计算三角形法线
            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 normal = normalize(cross(edge1, edge2));
            
            // 累加到三个顶点
            normals[i0] += normal;
            normals[i1] += normal;
            normals[i2] += normal;
        }
        
        // 标准化所有法线
        for (auto& normal : normals) {
            normal = normalize(normal);
        }
        
        return normals;
    }
    
    void buildVertexData(const std::vector<glm::vec3>& temp_vertices,
                        const std::vector<glm::vec2>& temp_uvs,
                        const std::vector<glm::vec3>& temp_normals,
                        const std::vector<unsigned int>& vertex_indices,
                        const std::vector<unsigned int>& uv_indices,
                        const std::vector<unsigned int>& normal_indices,
                        bool hasOriginalNormals) {
        
        vertices.clear();
        indices.clear();
        
        // 创建唯一的顶点
        for (size_t i = 0; i < vertex_indices.size(); i++) {
            Vertex vertex;
            
            // 位置
            vertex.Position = temp_vertices[vertex_indices[i]];
            
            // 纹理坐标
            if (i < uv_indices.size() && uv_indices[i] < temp_uvs.size()) {
                vertex.TexCoords = temp_uvs[uv_indices[i]];
            } else {
                vertex.TexCoords = glm::vec2(0.0f);
            }
            
            // 法线
            if (hasOriginalNormals && i < normal_indices.size()) {
                vertex.Normal = temp_normals[normal_indices[i]];
            } else {
                vertex.Normal = temp_normals[vertex_indices[i]];
            }
            
            vertices.push_back(vertex);
            indices.push_back(i);
        }
    }
    
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        
        // 顶点位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        
        // 法线
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);
        
        // 纹理坐标
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }
    
public:
    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    ~OBJLoader() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
    }
};

#endif