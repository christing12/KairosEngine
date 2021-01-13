#pragma pack_matrix(row_major)


struct VertexInput
{
    float3 position : POSITION;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 direction : TEXCOORD;
};


struct SkyboxConstants {
    float4x4 viewProjMat;
};
ConstantBuffer<SkyboxConstants> SkyboxCB : register(b0);


TextureCube skybox : register(t0);
SamplerState defaultSampler : register(s0);

PixelInput main_vs(VertexInput input)
{
    PixelInput output;

    output.position = mul(float4(input.position, 1.0f), SkyboxCB.viewProjMat).xyww;
    output.direction = input.position;
    return output;
}

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define PI_OVER_TWO 1.5707963268f
#define PI_SQUARED 9.86960440109f


float2 SphericalSample(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(-v.y)); // sphereical to cartesian
    uv /= float2(-TWO_PI, PI);
    uv += float2(0.5, 0.5);  // converts from from [-0.5, 0.5] -> [0, 1]
    return uv;
}

float4 main_ps(PixelInput input) : SV_TARGET
{
   // float2 uv = SphericalSample(normalize(input.direction));
    return skybox.Sample(defaultSampler, normalize(input.direction));
    //sreturn float4(0.25f, 0.75f, 1.0f, 1.0f);
    /*float3 envVector = normalize(input.direction);
    return skybox.Sample(s1, envVector);*/
}

