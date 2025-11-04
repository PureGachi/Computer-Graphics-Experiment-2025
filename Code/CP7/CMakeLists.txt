# CP6 子项目 - Z-Map 图形渲染器
# 第三方库已在父CMakeLists.txt中配置

# 设置第三方库路径（从父目录获取）
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)
set(GLFW_DIR ${THIRDPARTY_DIR}/glfw)
set(GLAD_DIR ${THIRDPARTY_DIR}/glad)
set(GLM_DIR ${THIRDPARTY_DIR}/glm)
set(STB_DIR ${THIRDPARTY_DIR}/stb)
set(IMGUI_DIR ${THIRDPARTY_DIR}/imgui)

# 包含目录
include_directories(
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
    ${GLM_DIR}
    ${STB_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ImGui源文件
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# 源文件
set(SOURCES
    src/sandbox.cpp
    src/camera.cpp
    src/shader.cpp
    src/tool.cpp
    src/workpiece.cpp
    src/cutter.cpp
    ${IMGUI_SOURCES}
)

# 头文件
set(HEADERS
    src/camera.hpp
    src/shader.hpp
    src/tool.hpp
    src/workpiece.hpp
    src/cutter.hpp
)

# 创建可执行文件
add_executable(ZMapRenderer ${SOURCES} ${HEADERS})

# 设置C++20标准
set_property(TARGET ZMapRenderer PROPERTY CXX_STANDARD 20)
set_property(TARGET ZMapRenderer PROPERTY CXX_STANDARD_REQUIRED ON)

# 链接库
target_link_libraries(ZMapRenderer 
    glfw
    glad
)

# Windows特定设置
if(WIN32)
    target_link_libraries(ZMapRenderer opengl32)
endif()

# 复制着色器文件到构建目录（多配置生成器）
add_custom_command(TARGET ZMapRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
    $<TARGET_FILE_DIR:ZMapRenderer>/shaders
    COMMENT "Copying shaders to build directory"
)

# 复制资源文件到构建目录（如果存在）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
    add_custom_command(TARGET ZMapRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:ZMapRenderer>/resources
        COMMENT "Copying resources to build directory"
    )
endif()

# 定义着色器路径宏
target_compile_definitions(ZMapRenderer PRIVATE
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/"
)

# 设置输出名称
set_target_properties(ZMapRenderer PROPERTIES
    OUTPUT_NAME "ZMapRenderer"
)
