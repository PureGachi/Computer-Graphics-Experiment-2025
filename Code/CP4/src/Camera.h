

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

// 相机移动方向
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 相机模式
enum Camera_Mode
{
    EULER,     // 欧拉角模式
    QUATERNION // 四元数模式
};

// 默认相机参数
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.0f;
const float SENSITIVITY = 0.05f;
const float FOV = 45.0f;

// 相机类 - 支持欧拉角和四元数两种模式
class Camera
{
  public:
    // 相机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 欧拉角（欧拉角模式使用）
    float Yaw;
    float Pitch;
    float Roll; // 添加 Roll 角度

    // 四元数（四元数模式使用）
    glm::quat Orientation;

    // 相机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Fov;

    // 相机模式
    Camera_Mode Mode;

    // 构造函数（使用向量）
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH, Camera_Mode mode = EULER)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Fov(FOV),
          Mode(mode), Roll(0.0f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 构造函数（使用标量）
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch,
           Camera_Mode mode = EULER)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Fov(FOV),
          Mode(mode), Roll(0.0f)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 切换相机模式
    void SetMode(Camera_Mode mode)
    {
        if (Mode != mode)
        {
            Mode = mode;

            if (mode == QUATERNION)
            {
                // 从欧拉角转换到四元数
                eulerToQuaternion();
            }
            else
            {
                // 从四元数转换到欧拉角
                quaternionToEuler();
            }
            updateCameraVectors();
        }
    }

    // 让相机朝向指定目标点
    void LookAt(glm::vec3 target)
    {
        glm::vec3 direction = glm::normalize(target - Position);

        // 计算对应的欧拉角
        // Pitch: 垂直旋转角度
        Pitch = glm::degrees(asin(direction.y));

        // Yaw: 水平旋转角度
        // direction.x = cos(yaw) * cos(pitch)
        // direction.z = sin(yaw) * cos(pitch)
        Yaw = glm::degrees(atan2(direction.z, direction.x));

        // 限制Pitch范围
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // 如果当前是四元数模式，直接从方向向量创建四元数
        if (Mode == QUATERNION)
        {
            // 使用GLM的lookAt旋转来直接创建四元数
            // 计算Right和Up向量
            glm::vec3 right = glm::normalize(glm::cross(direction, WorldUp));
            glm::vec3 up = glm::cross(right, direction);

            // 从旋转矩阵创建四元数
            glm::mat3 rotationMatrix;
            rotationMatrix[0] = right;
            rotationMatrix[1] = up;
            rotationMatrix[2] = -direction; // OpenGL使用-Z作为前方

            Orientation = glm::quat_cast(rotationMatrix);
            Orientation = glm::normalize(Orientation);
        }

        // 更新相机向量
        updateCameraVectors();
    }

    // 获取当前模式
    Camera_Mode GetMode() const
    {
        return Mode;
    }

    // 设置欧拉角并更新相机向量（用于ImGui控制）
    void SetEulerAngles(float yaw, float pitch, float roll = 0.0f, bool constrainPitch = false)
    {
        Yaw = yaw;
        Pitch = pitch;
        Roll = roll;

        // 可选：限制俯仰角
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // 设置四元数的欧拉角（Yaw, Pitch, Roll）
    void SetQuaternionEulerAngles(float yaw, float pitch, float roll)
    {
        Yaw = yaw;
        Pitch = pitch;

        // 从欧拉角创建四元数（使用 ZYX 顺序：Roll, Pitch, Yaw）
        glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat qRoll = glm::angleAxis(glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));

        // 组合旋转：Yaw * Pitch * Roll
        Orientation = qYaw * qPitch * qRoll;
        Orientation = glm::normalize(Orientation);

        updateCameraVectors();
    }

    // 使用轴角表示法设置四元数（避免万向锁的正确方式）
    void SetQuaternionAxisAngle(glm::vec3 axis, float angle)
    {
        // 确保轴是归一化的
        axis = glm::normalize(axis);

        // 从轴角创建四元数
        Orientation = glm::angleAxis(glm::radians(angle), axis);
        Orientation = glm::normalize(Orientation);

        updateCameraVectors();
    }

    // 应用增量旋转到当前四元数（避免万向锁的方式）
    void ApplyIncrementalRotation(glm::vec3 axis, float angleDelta)
    {
        // 确保轴是归一化的
        axis = glm::normalize(axis);

        // 创建增量旋转四元数
        glm::quat deltaQuat = glm::angleAxis(glm::radians(angleDelta), axis);

        // 应用增量旋转
        Orientation = deltaQuat * Orientation;
        Orientation = glm::normalize(Orientation);

        updateCameraVectors();
    }

    // 返回视图矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 处理键盘输入
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // 处理鼠标移动
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        if (Mode == EULER)
        {
            // 欧拉角模式
            Yaw += xoffset;
            Pitch += yoffset;

            // 限制俯仰角，防止翻转
            if (constrainPitch)
            {
                if (Pitch > 89.0f)
                    Pitch = 89.0f;
                if (Pitch < -89.0f)
                    Pitch = -89.0f;
            }
        }
        else
        {
            // 四元数模式
            // 创建旋转四元数（使用正确的旋转方向）
            glm::quat qPitch = glm::angleAxis(glm::radians(yoffset), Right);                      // 上下取正
            glm::quat qYaw = glm::angleAxis(glm::radians(-xoffset), glm::vec3(0.0f, 1.0f, 0.0f)); // 左右取负

            // 先应用俯仰，再应用偏航
            Orientation = qYaw * Orientation * qPitch;
            Orientation = glm::normalize(Orientation);
        }

        // 更新相机向量
        updateCameraVectors();
    }

    // 处理鼠标滚轮（可选，用于缩放）
    void ProcessMouseScroll(float yoffset)
    {
        Fov -= (float)yoffset;
        if (Fov < 1.0f)
            Fov = 1.0f;
        if (Fov > 45.0f)
            Fov = 45.0f;
    }

  private:
    // 根据相机模式更新向量
    void updateCameraVectors()
    {
        if (Mode == EULER)
        {
            // 欧拉角模式：从欧拉角计算方向向量（包括 Roll）
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            Front = glm::normalize(front);

            // 计算右向量和上向量
            Right = glm::normalize(glm::cross(Front, WorldUp));
            glm::vec3 tempUp = glm::normalize(glm::cross(Right, Front));

            // 应用 Roll 旋转（绕 Front 轴旋转）
            if (abs(Roll) > 0.001f)
            {
                glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Roll), Front);
                Right = glm::vec3(rollMatrix * glm::vec4(Right, 0.0f));
                Up = glm::vec3(rollMatrix * glm::vec4(tempUp, 0.0f));
            }
            else
            {
                Up = tempUp;
            }
        }
        else
        {
            // 四元数模式：从四元数计算方向向量
            glm::mat4 rotationMatrix = glm::toMat4(Orientation);

            // 从旋转矩阵提取基向量
            // GLM的矩阵是列主序，rotationMatrix[i]是第i列
            Right = glm::normalize(glm::vec3(rotationMatrix[0]));  // X轴（第0列）
            Up = glm::normalize(glm::vec3(rotationMatrix[1]));     // Y轴（第1列）
            Front = -glm::normalize(glm::vec3(rotationMatrix[2])); // -Z轴（第2列，取负因为相机朝向-Z）
        }
    }

    // 从欧拉角转换到四元数
    void eulerToQuaternion()
    {
        // 使用角度轴方法构建四元数
        // 先绕Y轴旋转Yaw，再绕局部X轴旋转Pitch
        glm::quat qYaw = glm::angleAxis(glm::radians(Yaw), glm::vec3(0.0f, 1.0f, 0.0f));

        // 计算旋转后的X轴（用于Pitch旋转）
        glm::vec3 localX = qYaw * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::quat qPitch = glm::angleAxis(glm::radians(Pitch), localX);

        // 组合两个旋转
        Orientation = qPitch * qYaw;
        Orientation = glm::normalize(Orientation);
    }

    // 从四元数转换到欧拉角
    void quaternionToEuler()
    {
        // 从四元数提取欧拉角（使用YXZ顺序）
        glm::vec3 euler = glm::eulerAngles(Orientation);

        // 转换为度数
        Pitch = glm::degrees(euler.x);
        Yaw = glm::degrees(euler.y);

        // 确保Yaw在合理范围内
        if (Yaw > 180.0f)
            Yaw -= 360.0f;
        else if (Yaw < -180.0f)
            Yaw += 360.0f;
    }
};
