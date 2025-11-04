# 设置第三方库路径（从父目录获取）
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)
set(GLFW_DIR ${THIRDPARTY_DIR}/glfw)
set(GLAD_DIR ${THIRDPARTY_DIR}/glad)
set(GLM_DIR ${THIRDPARTY_DIR}/glm)
set(TINYGLTF_DIR ${THIRDPARTY_DIR}/tinygltf)
set(STB_DIR ${THIRDPARTY_DIR}/stb)
set(IMGUI_DIR ${THIRDPARTY_DIR}/imgui)

# 包含目录
include_directories(
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
    ${GLM_DIR}
    ${TINYGLTF_DIR}
    ${STB_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 源文件
set(SOURCES
    src/main.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# 头文件
set(HEADERS
    src/Camera.h
    src/Utils.h
    src/GLBLoader.h
)

# 创建可执行文件
add_executable(CameraExperiment ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(CameraExperiment 
    glfw
    glad
)

# Windows特定设置
if(WIN32)
    target_link_libraries(CameraExperiment opengl32)
endif()

# 复制着色器文件到构建目录
add_custom_command(TARGET CameraExperiment POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
    $<TARGET_FILE_DIR:CameraExperiment>/shaders
    COMMENT "Copying shaders to build directory"
)

# 复制资源文件到构建目录
add_custom_command(TARGET CameraExperiment POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:CameraExperiment>/resources
    COMMENT "Copying resources to build directory"
)
