#include "Constants.hlsl"


struct VIn
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VOut
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};


VOut main(VIn vIn)
{
    VOut output;
    output.position = mul(ModelViewProjectionCB.MVP, float4(vIn.position, 1.0f));
    output.texCoord = vIn.texCoord;
    return output;
}