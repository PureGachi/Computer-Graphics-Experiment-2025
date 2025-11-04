#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// PBR材质参数
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// 纹理贴图
uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;

// 是否使用纹理
uniform bool useAlbedoMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useNormalMap;
uniform bool useAOMap;

// 光照参数
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 camPos;

const float PI = 3.14159265359;

// 法线映射
vec3 getNormalFromMap()
{
    if (!useNormalMap)
        return normalize(Normal);

    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(WorldPos);
    vec3 Q2 = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// 分布函数 (Normal Distribution Function)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// 几何函数 (Geometry Function)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 菲涅尔方程 (Fresnel Equation)
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 获取材质属性
    vec3 albedoValue = useAlbedoMap ? pow(texture(albedoMap, TexCoords).rgb, vec3(2.2)) : albedo;
    float metallicValue = useMetallicMap ? texture(metallicMap, TexCoords).r : metallic;
    float roughnessValue = useRoughnessMap ? texture(roughnessMap, TexCoords).r : roughness;
    float aoValue = useAOMap ? texture(aoMap, TexCoords).r : ao;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // 计算反射率F0
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedoValue, metallicValue);

    // 反射方程
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        // 计算每个光源的辐射度
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughnessValue);
        float G = GeometrySmith(N, V, L, roughnessValue);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallicValue;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedoValue / PI + specular) * radiance * NdotL;
    }

    // 环境光照
    vec3 ambient = vec3(0.03) * albedoValue * aoValue;

    vec3 color = ambient + Lo;

    // HDR色调映射和伽马校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}