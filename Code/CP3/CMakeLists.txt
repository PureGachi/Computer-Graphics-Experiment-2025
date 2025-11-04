# CP2 子项目 - FrameBuffer模糊效果
# 第三方库已在父CMakeLists.txt中配置

# 设置第三方库路径（从父目录获取）
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)
set(GLFW_DIR ${THIRDPARTY_DIR}/glfw)
set(GLAD_DIR ${THIRDPARTY_DIR}/glad)
set(GLM_DIR ${THIRDPARTY_DIR}/glm)

# 包含目录
include_directories(
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
    ${GLM_DIR}
)

# 源文件
set(SOURCES
    src/main.cpp
)

# 创建可执行文件
add_executable(FrameBufferBlur ${SOURCES})

# 链接库
target_link_libraries(FrameBufferBlur 
    glfw
    glad
)

# Windows特定设置
if(WIN32)
    target_link_libraries(FrameBufferBlur opengl32)
endif()

# 复制着色器文件到构建目录（多配置生成器）
add_custom_command(TARGET FrameBufferBlur POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
    $<TARGET_FILE_DIR:FrameBufferBlur>/shaders
    COMMENT "Copying shaders to build directory"
)
