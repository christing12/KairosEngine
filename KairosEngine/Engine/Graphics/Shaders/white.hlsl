//#include "Constants.hlsl"


struct VInput
{
    float3 position : POSITION;
};

struct PInput
{
    float4 position : SV_POSITION;
};

//
//float4 main_ps(VOut pIn) : SV_TARGET
//{
//    return float4(1.0, 1.0, 1.0, 1.0);
//}
