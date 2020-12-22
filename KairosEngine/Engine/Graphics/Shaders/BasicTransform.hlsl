#include "Constants.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float4 color : COLOR0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

VOut main(VIn vIn)
{


    VOut output;
    float3 pos = (mul(float4(vIn.position, 1.0f), MWMat)).xyz;
    output.position = mul(float4(pos, 1.0f), VP);

    output.color = vIn.color;

    return output;
}