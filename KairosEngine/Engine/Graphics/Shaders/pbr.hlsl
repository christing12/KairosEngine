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
    float2 texCoord : TEXCOORD;
    float3x3 tangentBasis : TBASIS;
};


VOut main_vs(VIn vIn)
{
    VOut output;
    output.worldPos = mul(float4(vIn.position, 1.0f), ModelCB.modelMat).xyz;
    output.position = mul(float4(output.worldPos, 1.0f), ModelCB.viewProjMat);

    float3x3 TBN = float3x3(vIn.tangent, vIn.bitangent, vIn.normal);
    output.tangentBasis = mul(transpose(TBN), (float3x3) ModelCB.modelMat);

    output.texCoord = vIn.texCoord;
    return output;
}


Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metalnessTexture : register(t2);
Texture2D roughnessTexture : register(t3);

SamplerState defaultSampler : register(s0);


static const float PI = 3.141592;
static const float Epsilon = 0.00001;

static const uint NumLights = 1;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


float4 main_ps(VOut pIn) : SV_TARGET
{
    float3 albedo = albedoTexture.Sample(defaultSampler, pIn.texCoord).rgb;
    float metalness = metalnessTexture.Sample(defaultSampler, pIn.texCoord).r;
    float roughness = roughnessTexture.Sample(defaultSampler, pIn.texCoord).r;

    //// outgoing light direction
    float3 Lo = normalize(CameraCB.cameraPos - pIn.worldPos);

    float3 N = normalize(2.0 * normalTexture.Sample(defaultSampler, pIn.texCoord).rgb - 1.0);
    N = normalize(mul(N, pIn.tangentBasis));

    float cosLo = saturate(dot(N, Lo));

    float3 R = 2.0 * cosLo * N - Lo;

    float3 F0 = lerp(Fdielectric, albedo, metalness);

    float3 directLighting = 0.0;


    float3 Li = float3(-1.0, 0.0, 0.0); // LIGHT CB DIRECTION
    float3 Lradiance = float3(1.0, 1.0, 1.0);

    // half vec
    float3 Lh = normalize(Li + Lo);

    float3 cosLi = max(0.0, dot(N, Li));
    float3 cosLh = max(0.0, dot(N, Lh));

    float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    float D = ndfGGX(cosLh, roughness);
    float G = gaSchlickGGX(cosLi, cosLo, roughness);

    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);

    float3 diffuseBRDF = kd * albedo;
    float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    directLighting = (diffuseBRDF + specularBRDF) * Lradiance * cosLi;

    return float4(directLighting, 1.0);
}