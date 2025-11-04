#include "camera.hpp"


void Camera::updateCameraVectors(){
    glm::vec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    //巧妙地利用世界坐标中的上方计算右侧向量
    right_ = glm::normalize(glm::cross(front_,worldup_));
    up_ = glm::normalize(glm::cross(right_,front_));
}

glm::vec3 Camera::GetViewFront(){
    return front_;
}

glm::vec3 Camera::GetViewPosition(){
    return position_;
}

glm::mat4 Camera::GetViewMatrix(){
    return glm::lookAt(position_,position_ + front_,up_);
}

void Camera::ProcessKeyboard(CameraMovement direction,float deltaTime){
    float velocity = moveSpeed_ * deltaTime;
    if(direction == CameraMovement::FORWARD){
        position_ += front_ * velocity;
    }
    if(direction == CameraMovement::BACKWARD){
        position_ -= front_ * velocity;
    }
    if(direction == CameraMovement::LEFT){
        position_ -= right_ * velocity;
    }
    if(direction == CameraMovement::RIGHT){
        position_ += right_ * velocity;
    }
}

void Camera::ProcessMouseMovement(float xoffset,float yoffset,GLboolean constrainpitch){
    xoffset *= mouseSensitivity_;
    yoffset *= mouseSensitivity_;
    yaw_ += xoffset;
    pitch_ += yoffset;

    if(constrainpitch){
        if(pitch_ > 89.0f){
            pitch_ = 89.0f;
        }
        if(pitch_ < -89.0f){
            pitch_ = -89.0f;
        }
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset){
    zoom_ -= yoffset;
    if(zoom_ < 1.0f){
        zoom_ = 1.0f;
    }
    if(zoom_ > 45.0f){
        zoom_ = 45.0f;
    }
}