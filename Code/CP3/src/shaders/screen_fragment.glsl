#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    // 模糊核大小
    const float offset = 1.0 / 300.0; // 可以调整这个值来改变模糊程度

    // 定义9个偏移位置（3x3核）
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // 左上
        vec2(0.0f, offset), // 正上
        vec2(offset, offset), // 右上
        vec2(-offset, 0.0f), // 左
        vec2(0.0f, 0.0f), // 中心
        vec2(offset, 0.0f), // 右
        vec2(-offset, -offset), // 左下
        vec2(0.0f, -offset), // 正下
        vec2(offset, -offset) // 右下
    );

    // 定义模糊核权重（高斯模糊）
    float kernel[9] = float[](
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
        2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0);

    // 应用模糊
    vec3 color = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        vec3 sampleTex = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        color += sampleTex * kernel[i];
    }

    FragColor = vec4(color, 1.0);
}
