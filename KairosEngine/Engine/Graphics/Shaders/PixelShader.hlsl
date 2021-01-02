#include "Constants.hlsl"

Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct VOut
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};


float4 main(VOut pIn) : SV_TARGET
{
    return LightDataCB.color;
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
    //return t1.Sample(s1, pIn.texCoord); 
}
