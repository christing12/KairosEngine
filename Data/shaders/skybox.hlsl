#include "Vertex.hlsl"
#include "Constants.hlsl"

struct SkyboxPass
{
    uint indexBufferOffset;
    uint vertexBufferOffset;
    uint skyboxTextureIndex;
    float4x4 skyboxViewProj;
};

#define PassDataType SkyboxPass

#include "EntryPoint.hlsl"

StructuredBuffer<Vertex> UnifiedVertexBuffer : register(t0);
StructuredBuffer<uint> UnifiedIndexBuffer : register(t1);

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 direction : TEXCOORD;
};

PixelInput main_vs(uint indexID : SV_VertexID)
{
    uint index = UnifiedIndexBuffer[indexID + PassDataCB.indexBufferOffset];
    Vertex vertex = UnifiedVertexBuffer[index + PassDataCB.vertexBufferOffset];
    
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
    return TexturesCube[PassDataCB.skyboxTextureIndex].Sample(LinearClampSampler(), normalize(input.direction));
}

