#include "Constants.hlsl"


struct VertexInput
{
    float3 position : POSITION;
};

struct PixelInput
{
    float4 position : SV_POSITION;
};


VOut main(VIn vIn)
{
    VOut output;
    output.position, = mul(float4(vIn.position, 1.0f), ModelCB.modelMat * ModelCB.viewProjMat);
    return output;
}

