#pragma pack_matrix(row_major)


struct VertexInput
{
    float3 position : POSITION;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 direction : POSITION;
};


struct SkyboxConstants {
    float4x4 projMat;
};
ConstantBuffer<SkyboxConstants> SkyboxCB : register(b0);


TextureCube skybox : register(t0);
SamplerState s1 : register(s0);

PixelInput main_vs(VertexInput input)
{
    PixelInput output;

    output.position = mul(SkyboxCB.projMat, float4(input.position, 1.0f));

    output.position.z = output.position.w;

    output.direction = input.position;

    return output;
}


float4 main_ps(PixelInput input) : SV_TARGET
{
    float3 envVector = normalize(input.direction);
    return skybox.Sample(s1, envVector);
}

