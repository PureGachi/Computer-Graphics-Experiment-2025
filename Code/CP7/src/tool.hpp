
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "camera.hpp"
#include "workpiece.hpp"
#include "cutter.hpp"

//方向、移动距离
struct Toolpath{
    glm::vec3 direction;
    int length;
};

const float width = 1200.0;
const float height = 1200.0;
// 声明外部变量
extern float deltaTime;
extern float lastFrame;
extern bool firstMouse;
extern float lastX;
extern float lastY;
extern bool isNeedUpdate;
extern double currentUpdateTime;
extern double lastUpdateTime;
extern Camera myCamera;
extern glm::mat4 projection;
extern std::vector<Toolpath> myPath;
extern int indices;

extern glm::mat4 cutterModelMatrix;

// 回调函数声明
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xPos, double yPos);
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);
void processInput(GLFWwindow *window);
void initWorkpieceData(WorkPiece &data);
void initWorkPieceRenderdata(std::vector<GLuint>& workGL,WorkPiece& workpiece);
void initCutterRenderdata(std::vector<GLuint>& cutterGL,Cutter& myCutter);
void updateZmap(WorkPiece &workpiece, Cutter &cutter,Toolpath myPath,glm::vec3 &toolPosition);
inline glm::vec3 getTranslateVec(Toolpath path,float precision){
    return glm::vec3(path.direction.x * path.length * precision,path.direction.y * path.length * precision,path.direction.z * path.length * precision);
}