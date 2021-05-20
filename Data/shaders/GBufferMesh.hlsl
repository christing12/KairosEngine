

#include "EntryPoint.hlsl

StructuredBuffer<Vertex> UnifiedVertexBuffer : register(t0);
StructuredBuffer<uint> UnifiedIndexBuffer : register(t1);
StructuredBuffer<Material> MaterialTable : register(t2);


struct PSInput
{
	float4 pixelPosition : SV_POSITION;
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3x3 tangentBasis : TBASIS;
};


PSInput main_vs(uint indexID : SV_VertexID)
{
	PSInput output;

	uint index = UnifiedIndexBuffer[indexID];
	Vertex vertex = UnifiedVertexBuffer[index];

	output.pixelPosition = mul(float4(vertex.position, 1.0), FrameDataCB.CurrCamera.ViewProjection);
	output.texcoord = vertex.texCoord;

	output.position = vertex.position;

	// Pass tangent space basis vectors (for normal mapping).
	float3x3 TBN = float3x3(vertex.tangent, vertex.bitangent, vertex.normal);
	output.tangentBasis = TBN;  //mul(transpose(TBN), (float3x3)modelMatrix);

	//float4x4 mvpMatrix = mul(modelMatrix, viewProjectionMatrix);
	//vout.pixelPosition = mul(float4(vin.position, 1.0), mvpMatrix);
	return output;
}
