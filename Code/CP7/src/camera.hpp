#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


class Camera{
private:    
    //摄像机属性
    glm::vec3 position_ = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 right_;
    glm::vec3 up_ = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 front_ = glm::vec3(0.0f,0.0f,-1.0f);
    glm::vec3 worldup_ = glm::vec3(0.0f,1.0f,0.0f);
    //欧拉角
    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    //摄像机选项
    //摄像机移动速度
    float moveSpeed_ = 2.5f;
    //鼠标灵敏度
    float mouseSensitivity_ = 0.1f;
    float zoom_ = 45.0f;

    //利用俯仰和左右角度来更新front向量，再更新上和右向量
    void updateCameraVectors();

public:

    Camera(glm::vec3 position,glm::vec3 up,float yaw,float pitch){
        position_ = position;
        up_ = up;
        yaw_ = yaw;
        pitch_ = pitch;
        updateCameraVectors();
    }
    glm::vec3 GetViewFront();
    glm::vec3 GetViewPosition();
    //返回视角矩阵
    glm::mat4 GetViewMatrix();
    //根据所获得的键盘输入，进行视角矩阵的更新
    void ProcessKeyboard(CameraMovement direction,float deltaTime);
    //根据所获得的鼠标输入，进行视角矩阵的更新
    void ProcessMouseMovement(float xoffset,float yoffset,GLboolean constrainpitch = true);
    //根据所获得的鼠标滚轮的输入，进行视角矩阵的更新
    void ProcessMouseScroll(float yoffset);
    float GetZoom(){
        return zoom_;
    }

};