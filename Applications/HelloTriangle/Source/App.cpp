#include "apppch.h"
#include "App.h"

extern Kairos::IEngine* g_Engine;

using namespace Kairos;
using namespace DirectX;

struct TransformCB
{
	Matrix modelMatrix;
	Matrix viewProjectionMatrix;
};

struct ShadingCB
{
	struct {
		Vector3 direction;
		float padding;
		Vector3 radiance;
		float padding1;
	} lights[3];
	Vector3 eyePosition;
	float padding;
};


D3D12_INPUT_ELEMENT_DESC PBRLayout[] = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};





bool App::Initialize()
{
	ApplicationEntry::Initialize();

	SetupResources();


	return true;
	
}

void App::Update()
{
	mCamera.Update(0.01f);
	//m_CameraData.viewProjMat = mCamera.GetViewProjMat();
	//m_CameraData.cameraPos = mCamera.GetPos();
	//cbSkybox.viewRotProjMat = mCamera.GetViewRotMat() * mCamera.GetProjMat();

	{
		Matrix rotXMat = Matrix::CreateRotationX(0.001f);
		Matrix rotYMat = Matrix::CreateRotationY(0.002f);
		Matrix rotZMat = Matrix::CreateRotationZ(0.0003f);

		Matrix rotMat = cube1RotMat * rotXMat * rotYMat * rotZMat;
		cube1RotMat = rotMat;

		Matrix translationMat = Matrix::CreateTranslation(cube1Position);

		Matrix worldMat = rotMat * translationMat;
		//cube1WorldMat = worldMat;
	}
}

void App::Render()
{
	//g_Engine->GetRenderBackend()->Present();
}

bool App::Shutdown()
{
	ApplicationEntry::Shutdown();
	return false;
}

bool App::OnEvent(Kairos::Event& e)
{
	ApplicationEntry::OnEvent(e);
	mCamera.OnEvent(e);

	return true;
}

void App::SetupResources() {

	projectDir = Filesystem::GetRelativePath("C:/Users/Chris Ting/Desktop/MeshRenderer/Data/");
	KRS_CORE_INFO(projectDir);


	float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
	std::wstring filePath = L"Data/shaders/";

	const  D3D12_INPUT_ELEMENT_DESC skyboxLayout[] = {
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


	CD3DX12_STATIC_SAMPLER_DESC defaultSamplerDesc{ 0, D3D12_FILTER_ANISOTROPIC };
	defaultSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };

	CD3DX12_STATIC_SAMPLER_DESC spBRDF_SamplerDesc{ 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	spBRDF_SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	spBRDF_SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	spBRDF_SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



	auto backend = g_Engine->GetRenderBackend();

	// PBR Pipeline 
	//{
	//	Ref<RootSignature> PBRSignature = backend->AllocateRootSignature("PBR", 3, 2);
	//	Ref<GraphicsPSO> pbrPSO = backend->AllocateGraphicsPSO("PBR");

	//	(*PBRSignature)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	//	(*PBRSignature)[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	//	(*PBRSignature)[2].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
	//	(*PBRSignature)[2].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0, 0);
	//	PBRSignature->SetStaticSampler(0, defaultSamplerDesc);
	//	PBRSignature->SetStaticSampler(1, spBRDF_SamplerDesc);
	//	PBRSignature->Finalize(L"PBR Root Signature", rootSignatureFlags);

	//	ShaderCreateInfo Vinfo(ShaderType::Vertex, filePath + L"pbr.hlsl", "main_vs");
	//	Ref<Shader> VS = CreateRef<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(), Vinfo);

	//	ShaderCreateInfo PInfo(ShaderType::Pixel, filePath + L"pbr.hlsl", "main_ps");
	//	Ref<Shader> PS = CreateRef<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(), PInfo);

	//	pbrPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ PBRLayout, _countof(PBRLayout) });
	//	pbrPSO->SetRootSignature(*PBRSignature);
	//	pbrPSO->SetVertexShader(VS);
	//	pbrPSO->SetPixelShader(PS);
	//	pbrPSO->SetRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
	//	pbrPSO->Finalize();
	//}


	{

		// set starting camera state
		cameraPosition = Vector3(0.0f, 0.0f, -5.0f);
		cameraTarget = Vector3(0.0f, 0.0f, 0.0f);
		cameraUp = Vector3(0.0f, 1.0f, 0.0f);


		mCamera = EditorCamera(XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f), cameraPosition);

		cube1Position = Vector3(0.f, 0.f, 0.f);
		cube1WorldMat = Matrix::CreateTranslation(cube1Position);
		cube1RotMat = Matrix::Identity;
	}
}