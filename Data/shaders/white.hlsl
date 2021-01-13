#include "Constants.hlsl"



struct VIn {
    float3 position : POSITION;
};

struct VOut {
    float4 position : SV_POSITION;
};

VOut main_vs(VIn vIn)
{
    VOut output;
    float3 worldPos = mul(float4(vIn.position, 1.0f), ModelCB.modelMat).xyz;
    output.position = mul(float4(worldPos, 1.0f), ModelCB.viewProjMat);
    // output.position = float4(1.0, 1.0, 1.0, 1.0);
    return output;
}
float4 main_ps(VOut input) : SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}

