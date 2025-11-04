#include "tool.hpp"

// 初始化外部变量（确保它们在一个 .cpp 文件中定义）
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = width / 2;
float lastY = height / 2;
bool isNeedUpdate = false;
Camera myCamera(glm::vec3(1.0, 2.5, 1.0), glm::vec3(0.0, 1.0, 0.0), 60.0f, 0.0f);
glm::mat4 projection = glm::perspective(glm::radians(myCamera.GetZoom()), (float)width / (float)height, 0.1f, 100.0f);
double currentUpdateTime = 0.0;
double lastUpdateTime = 0.0;
glm::mat4 cutterModelMatrix = glm::mat4(1.0);


std::vector<Toolpath> myPath = {
    {glm::vec3(1.0f, -0.5f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 1.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(1.0f, 0.0f, 0.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
    {glm::vec3(0.0f, 0.0f, 1.0f), 5},
};

int indices = 0;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xPos, double yPos)
{
    float currentX = static_cast<float>(xPos);
    float currentY = static_cast<float>(yPos);

    if (firstMouse)
    {
        lastX = currentX;
        lastY = currentY;
        firstMouse = false;
    }

    float xoffset = currentX - lastX;
    float yoffset = lastY - currentY;
    lastX = currentX;
    lastY = currentY;
    myCamera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset)
{
    myCamera.ProcessMouseScroll(static_cast<float>(yOffset));
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        isNeedUpdate = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        myCamera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        myCamera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        myCamera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        myCamera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    }
}

void initWorkpieceData(WorkPiece &workpiece)
{
    for (int x = 0; x < workpiece.length; x++)
    {
        for (int z = 0; z < workpiece.width; z++)
        {
            workpiece.setDepth(x, z, 0.0);
        }
    }
}

void initWorkPieceRenderdata(std::vector<GLuint> &workGL, WorkPiece &workpiece)
{
    glBindVertexArray(workGL[0]);
    glBindBuffer(GL_ARRAY_BUFFER, workGL[2]);
    glBufferData(GL_ARRAY_BUFFER, workpiece.zmapCoords.size() * sizeof(float), workpiece.zmapCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, workGL[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, workpiece.zmapIndices.size() * sizeof(int), workpiece.zmapIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(workGL[1]);
    glBindBuffer(GL_ARRAY_BUFFER, workGL[4]);
    glBufferData(GL_ARRAY_BUFFER, workpiece.zmapCoords.size() * sizeof(float), workpiece.zmapCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, workGL[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, workpiece.lineIndices.size() * sizeof(int), workpiece.lineIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

void initCutterRenderdata(std::vector<GLuint> &cutterGL, Cutter &myCutter)
{
    glBindVertexArray(cutterGL[0]);
    glBindBuffer(GL_ARRAY_BUFFER, cutterGL[2]);
    glBufferData(GL_ARRAY_BUFFER, myCutter.ballCoords.size() * sizeof(float), myCutter.ballCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cutterGL[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCutter.ballIndices.size() * sizeof(int), myCutter.ballIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(cutterGL[1]);
    glBindBuffer(GL_ARRAY_BUFFER, cutterGL[4]);
    glBufferData(GL_ARRAY_BUFFER, myCutter.ballCoords.size() * sizeof(float), myCutter.ballCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cutterGL[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCutter.balllineIndices.size() * sizeof(int), myCutter.balllineIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

void updateZmap(WorkPiece &workpiece, Cutter &cutter, Toolpath myPath, glm::vec3 &toolPosition)
{
    int wpwidth = workpiece.width;
    int wplength = workpiece.length;
    int cwidth = cutter.width;
    int clength = cutter.length;
    for (int m = 0; m < myPath.length; m++)
    {
        for (int i = 0; i < cwidth; i++)
        {
            for (int j = 0; j < clength; j++)
            {
                if ((i + toolPosition.x + myPath.direction.x * m + 1 > wpwidth) || (j + toolPosition.z + myPath.direction.z * m + 1 > wplength) || (j + toolPosition.z + myPath.direction.z * m + 1 <= 0) || (i + toolPosition.x + myPath.direction.x * m + 1 <= 0))
                {
                    continue;
                }
                if (workpiece.depthData[(i + toolPosition.x + myPath.direction.x * m) * wplength + toolPosition.z + j + myPath.direction.z * m] > (cutter.depthData[i * clength + j] + (toolPosition.y + myPath.direction.y*m) * cutter.precision))
                {
                    workpiece.depthData[(i + toolPosition.x + myPath.direction.x * m) * wplength + toolPosition.z + j + myPath.direction.z * m] = cutter.depthData[i * clength + j] + (toolPosition.y + myPath.direction.y*m) * cutter.precision;
                }
            }
        }
    }
    toolPosition = toolPosition + glm::vec3(myPath.direction.x * myPath.length, myPath.direction.y * myPath.length, myPath.direction.z * myPath.length);
}