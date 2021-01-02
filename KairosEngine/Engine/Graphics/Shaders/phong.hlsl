#include "Constants.hlsl"

struct VIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 texCoord : TEXCOORD;
};

struct VOut
{
    float3 worldPos : POSITION;
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD;
};


VOut main_vs(VIn vIn)
{
    VOut output;
    output.worldPos = mul(float4(vIn.position, 1.0f), ModelCB.modelMat).xyz;
    output.position = mul(float4(output.worldPos, 1.0f), ModelCB.viewProjMat);

    output.normal = mul(float4(vIn.normal, 0.0f), ModelCB.modelMat).xyz;
    output.tangent = mul(float4(vIn.tangent, 0.0f), ModelCB.modelMat).xyz;

    output.texCoord = vIn.texCoord;
    return output;
}


Texture2D t1 : register(t0);
Texture2D t2 : register(t1);
Texture2D t3 : register(t2);
Texture2D t4 : register(t3);

SamplerState s1 : register(s0);

float4 main_ps(VOut pIn) : SV_TARGET
{
    float3 finalColor = float3(0.5, 0.5, 0.5);

    float3 viewDir = normalize(CameraCB.cameraPos - pIn.worldPos);
    float3 norm = normalize(pIn.normal);

    float3 lightDir = normalize(LightDataCB.position - pIn.worldPos);
    float3 reflectVec = -1 * normalize(reflect(lightDir, norm));

    float3 diffuse = saturate(dot(norm, lightDir)) * LightDataCB.diffuseColor;
    float3 specular = pow(saturate(dot(reflectVec, viewDir)), LightDataCB.specular)
        * LightDataCB.specularColor;

    finalColor += diffuse;
    finalColor += specular;

    float3 texelColor = t1.Sample(s1, pIn.texCoord).xyz;

    return float4(texelColor, 1.0f);
}