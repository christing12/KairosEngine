
struct VertexInput {
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

PixelInput main_vs(VertexInput input)
{
	PixelInput output;

	output.position = float4(input.position, 1.0);
	output.texCoord = input.texCoord;
	return output;
}


Texture2D tex : register(t0);
SamplerState defaultSampler : register(s0);


float4 main_ps(PixelInput input) : SV_TARGET
{
	return tex.Sample(defaultSampler, input.texCoord);
}