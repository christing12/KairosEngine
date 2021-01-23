#include "EntryPoint.hlsl"
#include "Constants.hlsl"
#include "Lighting.hlsl"
#include "Vertex.hlsl"

struct IndexU32
{
	uint Index;
};

struct Material
{
    uint AlbedoMapIndex;
    uint NormalMapIndex;
	uint MetalnessMapIndex;
    uint RoughnessMapIndex;
};

struct Light
{
	float4 radiance;
	float3 direction;
};

struct RootConstants
{
	uint InstanceTableIndex;
};

//ConstantBuffer<RootConstants> RootConstantsBuffer : register(b0);
StructuredBuffer<Vertex> UnifiedVertexBuffer : register(t0);
StructuredBuffer<uint> UnifiedIndexBuffer : register(t1);
StructuredBuffer<Material> MaterialTable : register(t2);
StructuredBuffer<Light> LightTable : register(t3);

//StructuredBuffer<MeshInstance> InstanceTable : register(t3);


struct VSInput
{
	float3 position : POSITION;
};

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

	output.pixelPosition = mul(float4(vertex.position, 1.0), FrameDataCB.ViewProjMat);
	output.texcoord = vertex.texCoord;

	output.position = vertex.position;

	// Pass tangent space basis vectors (for normal mapping).
	float3x3 TBN = float3x3(vertex.tangent, vertex.bitangent, vertex.normal);
	output.tangentBasis = TBN;  //mul(transpose(TBN), (float3x3)modelMatrix);

	//float4x4 mvpMatrix = mul(modelMatrix, viewProjectionMatrix);
	//vout.pixelPosition = mul(float4(vin.position, 1.0), mvpMatrix);
	return output;
}

static const uint NumLights = 3;

float4 main_ps(PSInput input) : SV_TARGET
{
	Material mat = MaterialTable[0];
	float4 result = Textures2D[mat.AlbedoMapIndex].Sample(LinearClampSampler(), input.texcoord);


	// Sample input textures to get shading model params.
	float3 albedo = Textures2D[mat.AlbedoMapIndex].Sample(LinearClampSampler(), input.texcoord).rgb;
	float metalness = Textures2D[mat.MetalnessMapIndex].Sample(LinearClampSampler(), input.texcoord).r;
	float roughness = Textures2D[mat.RoughnessMapIndex].Sample(LinearClampSampler(), input.texcoord).r;

	float3 eyePosition = FrameDataCB.EyePosition;
	// Outgoing light direction (vector from world-space fragment position to the "eye").
	float3 Lo = normalize(FrameDataCB.EyePosition - input.position);

	// Get current fragment's normal and transform to world space.
	float3 N = normalize(2.0 * Textures2D[mat.NormalMapIndex].Sample(LinearClampSampler(), input.texcoord).rgb - 1.0);
	N = normalize(mul(input.tangentBasis, N));

	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));

	// Specular reflection vector.
	float3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	float3 F0 = lerp(Fdielectric, albedo, metalness);

	// Direct lighting calculation for analytical lights.
	float3 directLighting = 0.0;

	uint numStructs = 0;
	uint stride = 0;
	LightTable.GetDimensions(numStructs, stride);

	for (uint i = 0; i < NumLights; ++i)
	{
		float3 Li = -LightTable[i].direction;
		float3 Lradiance = (LightTable[i].radiance).xyz;

		// Half-vector between Li and Lo.
		float3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);


		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		float3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}

	float3 ambientLighting = float3(0.1, 0.1, 0.1);

	return float4(directLighting, 1.0);
}
