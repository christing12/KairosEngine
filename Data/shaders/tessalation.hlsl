#include "Vertex.hlsl"


StructuredBuffer<Vertex> vinput : register(b0);
StructuredBuffer<uint> iinput : register(b1);

struct HullInput
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

HullInput main_vs(uint indexID : SV_VertexID)
{
	HullInput output;
	Vertex vertex = vinput[iinput[indexID]];
	output.position = vertex.position;
	output.texcoord = vertex.texcoord;
	return output;
}

// Hull Shader
struct DomainInput
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct TessellationPatch
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

//TessellationPatch main_hs(
//
//)
