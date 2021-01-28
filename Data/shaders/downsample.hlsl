#include "EntryPoint.hlsl"

static const float gamma = 2.2;

struct DownsampleConstants
{
	uint inputTextureIndex;
	uint outputTextureIndex;
};

ConstantBuffer<DownsampleConstants> textureConstants : register(b0);

[numthreads(8, 8, 1)]
void downsample_linear(uint ThreadID : SV_DispatchThreadID)
{
	int3 sampleLocation = int3(2 * ThreadID.x, 2 * ThreadID.y, 0);
	float4 gatherValue =
		Textures2D[inputTextureIndex].Load(sampleLocation, int2(0, 0)) +
		Textures2D[inputTextureIndex].Load(sampleLocation, int2(1, 0)) +
		Textures2D[inputTextureIndex].Load(sampleLocation, int2(0, 1)) +
		Textures2D[inputTextureIndex].Load(sampleLocation, int2(1, 1));
	RW_Float4_Textures2D[outputTextureIndex][ThreadID] = 0.25 * gatherValue;
}


[numthreads(8, 8, 1)]
void downsample_gamma(uint2 ThreadID : SV_DispatchThreadID)
{
	int3 sampleLocation = int3(2 * ThreadID.x, 2 * ThreadID.y, 0);

	float4 value0 = Textures2D[inputTextureIndex].Load(sampleLocation, int2(0, 0));
	float4 value1 = Textures2D[inputTextureIndex].Load(sampleLocation, int2(1, 0));
	float4 value2 = Textures2D[inputTextureIndex].Load(sampleLocation, int2(0, 1));
	float4 value3 = Textures2D[inputTextureIndex].Load(sampleLocation, int2(1, 1));

	float4 gatherValue;
	gatherValue.rgb = pow(value0.rgb, gamma) + pow(value1.rgb, gamma) + pow(value2.rgb, gamma) + pow(value3.rgb, gamma);
	gatherValue.a = value0.a + value1.a + value2.a + value3.a;

	RW_Float4_Textures2D[outputTextureIndex][ThreadID] = float4(pow(0.25 * gatherValue.rgb, 1.0 / gamma), 0.25 * gatherValue.a);
}


[numthreads(8, 8, 1)]
void downsample_array_linear(uint3 ThreadID : SV_DispatchThreadID)
{
	int4 sampleLocation = int4(2 * ThreadID.x, 2 * ThreadID.y, ThreadID.z, 0);
	float4 gatherValue =
		Textures2D[inputTextureIndex].Load(sampleLocation, int3(0, 0, 0)) +
		Textures2D[inputTextureIndex].Load(sampleLocation, int3(1, 0, 0)) +
		Textures2D[inputTextureIndex].Load(sampleLocation, int3(0, 1, 0)) +
		Textures2D[inputTextureIndex].Load(sampleLocation, int3(1, 1, 0));
	RW_Float4_Texture2DArrays[outputTextureIndex][ThreadID] = 0.25 * gatherValue;
}
