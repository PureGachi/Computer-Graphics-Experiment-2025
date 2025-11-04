#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 先应用模型变换旋转球体
    vec4 rotatedPos = model * vec4(aPos, 1.0);

    // 使用旋转后的位置
    FragPos = rotatedPos.xyz;
    TexCoord = aTexCoord;

    // 移除 view 矩阵的平移部分，让天空盒跟随相机
    mat4 viewNoTranslation = mat4(mat3(view));
    vec4 pos = projection * viewNoTranslation * rotatedPos;

    // 确保天空盒总是在最远处
    gl_Position = pos.xyww;
}
