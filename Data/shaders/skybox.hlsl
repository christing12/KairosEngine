#include "Vertex.hlsl"
#include "Constants.hlsl"

struct SkyboxPass
{
    float4x4 skyboxViewProj;
    uint skyboxTextureIndex;
};

#define PassDataType SkyboxPass

#include "EntryPoint.hlsl"

StructuredBuffer<Vertex> CubeMesh : register(t0);
StructuredBuffer<uint> CubeIndices : register(t1);

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 direction : TEXCOORD;
};

PixelInput main_vs(uint indexID : SV_VertexID)
{
    uint index = CubeIndices[indexID];
    Vertex vertex = CubeMesh[index];
    
    PixelInput output;
    output.position = mul(float4(vertex.position, 1.0), PassDataCB.skyboxViewProj).xyww;
    output.direction = vertex.position;

    return output;
    
}


float2 SphericalSample(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(-v.y)); // sphereical to cartesian
    uv /= float2(-TwoPi, Pi);
    uv += float2(0.5, 0.5);  // converts from from [-0.5, 0.5] -> [0, 1]
    return uv;
}

float4 main_ps(PixelInput input) : SV_TARGET
{
    return TexturesCube[PassDataCB.skyboxTextureIndex].Sample(AnisotropicClampSampler(), normalize(input.direction));
}

