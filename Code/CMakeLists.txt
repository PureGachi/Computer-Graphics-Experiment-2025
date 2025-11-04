cmake_minimum_required(VERSION 3.10)
project(GraphicsProjects)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置第三方库路径
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

# GLFW
set(GLFW_DIR ${THIRDPARTY_DIR}/glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${GLFW_DIR})

# GLAD
set(GLAD_DIR ${THIRDPARTY_DIR}/glad)
add_library(glad ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# GLM
set(GLM_DIR ${THIRDPARTY_DIR}/glm)

# 添加子项目
add_subdirectory(CP1)
add_subdirectory(CP2)
add_subdirectory(CP3)
add_subdirectory(CP4)
add_subdirectory(CP5)
add_subdirectory(CP6)
add_subdirectory(CP7)