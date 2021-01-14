#include "EntryPoint.hlsl"

struct VSInput
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct PSInput
{
	float4 pixelPos : SV_POSITION;
};


PSInput main_vs(VSInput input)
{
	PSInput output;
	output.pixelPos = float4(1.0, 1.0, 1.0, 1.0);
	return output;
}


float4 main_ps(PSInput input) : SV_TARGET
{
	return float4(1.0, 1.0, 1.0, 1.0);
}
