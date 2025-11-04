#version 330 core
in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
    // 如果有纹理坐标，使用纹理
    if (TexCoord.x >= 0.0 && TexCoord.y >= 0.0) {
        FragColor = texture(texture0, TexCoord);
    } else {
        // 使用位置生成渐变色（程序化天空）
        vec3 normalized = normalize(FragPos);
        vec3 color = normalized * 0.5 + 0.5;
        FragColor = vec4(color, 1.0);
    }
}
