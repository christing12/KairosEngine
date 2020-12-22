

static const float gamma = 2.2;
static const float exposure = 1.0;
static const float pureWhite = 1.0;

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

Texture2D sceneColor : register(t0);
SamplerState defaultSampler : register(s0);

PixelShaderInput main_vs(uint vertexID : SV_VertexID)
{
	PixelShaderInput vout;
	if (vertexID == 0) {

	}
}