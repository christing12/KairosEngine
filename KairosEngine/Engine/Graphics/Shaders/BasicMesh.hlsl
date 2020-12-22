#include "Constants.hlsl"


struct VIn
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 textureUV : TEXCOORD;
    float3 normal : NORMAL0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 textureUV : TEXCOORD;

    float3 normal : NORMAL0;
    float3 worldPos: POSITION0;
};

VOut VS(VIn vIn)
{
    VOut output;
    output.worldPos = (mul(float4(vIn.position, 1.0f), c_modelToWorld)).xyz;
    output.position = mul(float4(output.worldPos, 1.0f), c_viewProj);
    output.normal = mul(float4(vIn.normal, 1.0f), c_modelToWorld).xyz;

    output.color = vIn.color;
    output.textureUV = vIn.textureUV;

    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    float3 V = normalize(c_cameraPosition - pIn.worldPos);
    float3 N = normalize(pIn.normal);
    float3 finalColor = c_ambient;
    for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
        PointLightData pointLight = c_pointLight[i];
        if (pointLight.isEnabled) {
            float3 lightVec = pointLight.position - pIn.worldPos;
            float3 L = normalize(lightVec);
            float3 R = -1 * normalize(reflect(L, N));


            float3 lightDist = length(lightVec);
            float falloff = smoothstep(-pointLight.outerRadius, -pointLight.innerRadius, -lightDist);
            float3 diffuse = saturate(dot(N, L)) * pointLight.diffuseColor * falloff;
            float3 specular = pow(saturate(dot(R, V)), pointLight.specularPower) * pointLight.specularColor * falloff;
            finalColor += diffuse + specular;
        }
    }
    float3 texelColor = (DiffuseTexture.Sample(DefaultSampler, pIn.textureUV) * pIn.color).xyz;
    return float4(texelColor * finalColor, 1.0f);
}
