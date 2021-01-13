#pragma pack_matrix(row_major)

static const float PI = 3.141592;
static const float Epsilon = 0.00001;

static const uint NumLights = 3;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

cbuffer TransformConstants : register(b0)
{
	float4x4 modelMatrix;
	float4x4 viewProjectionMatrix;
};

cbuffer ShadingConstants : register(b0)
{
	struct {
		float3 direction;
		float padding;
		float3 radiance;
		float padding1;
	} lights[NumLights];
	float3 eyePosition;
};

struct VertexShaderInput
{
	float3 position  : POSITION;
	float3 normal    : NORMAL;
	float3 tangent   : TANGENT;
	float3 bitangent : BITANGENT;
	float2 texcoord  : TEXCOORD;
};
struct PixelShaderInput
{
	float4 pixelPosition : SV_POSITION;
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3x3 tangentBasis : TBASIS;
};

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metalnessTexture : register(t2);
Texture2D roughnessTexture : register(t3);
TextureCube specularTexture : register(t4);
TextureCube irradianceTexture : register(t5);
Texture2D specularBRDF_LUT : register(t6);

SamplerState defaultSampler : register(s0);

// Vertex shader
PixelShaderInput main_vs(VertexShaderInput vin)
{
	PixelShaderInput vout;
	vout.position = mul(float4(vin.position, 1.0), modelMatrix).xyz;
	vout.texcoord = float2(vin.texcoord.x, 1.0 - vin.texcoord.y);

	// Pass tangent space basis vectors (for normal mapping).
	float3x3 TBN = float3x3(vin.tangent, vin.bitangent, vin.normal);
	vout.tangentBasis = mul(transpose(TBN), (float3x3)modelMatrix);

	float4x4 mvpMatrix = mul(modelMatrix, viewProjectionMatrix);
	vout.pixelPosition = mul(float4(vin.position, 1.0), mvpMatrix);
	return vout;
}

// Pixel shader
float4 main_ps(PixelShaderInput pin) : SV_Target
{
	// Sample input textures to get shading model params.
	float3 albedo = albedoTexture.Sample(defaultSampler, pin.texcoord).rgb;

	float3 finalColor = float3(0.1, 0.1, 0.1);

	float3 viewDir = normalize(eyePosition - pin.position);
	float3 N = normalize(2.0 * normalTexture.Sample(defaultSampler, pin.texcoord).rgb - 1.0);

	for (int i = 0; i < 1; i++) {
		float3 reflectVec = reflect(-1 * lights[i].direction, N);

		float lambertian = saturate(dot(N, lights[i].direction));
		float3 diffuse = lambertian * lights[i].radiance;

		float3 specular = pow(saturate(dot(reflectVec, viewDir)), 32)
			* float3(1.0, 1.0, 1.0);

		finalColor += diffuse;
		finalColor += specular;
	}


	return float4(finalColor * albedo, 1.0f);
}
