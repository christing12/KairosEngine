#include "apppch.h"
#include "App.h"

extern Kairos::Engine* g_Engine;

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

bool App::Initialize()
{
	ApplicationEntry::Initialize();


	for (int frame = 0; frame < 2; ++frame)
	{
		mFrameBuffers[frame].ColorBuffer = m_Engine->GetRenderBackend()->GetRenderDevice()->CreateTexture(TextureProperties(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, TextureType::Tex2D), ResourceState::RenderTarget);
		mFrameBuffers[frame].DepthBuffer = m_Engine->GetRenderBackend()->GetRenderDevice()->CreateTexture(TextureProperties(1024, 1024, DXGI_FORMAT_D24_UNORM_S8_UINT, TextureType::Tex2D), ResourceState::DepthWrite);
	}


	SetupResources();


	return true;
	
}

void App::Update()
{
	m_Scene.Update(0.01f);

	mGlobalConstants.LinearClampSamplerIdx = linearSamplerHandle.Index();
	mGlobalConstants.AnisotropicClampSamplerIdx = aniSamplerHandler.Index();
	mGlobalConstants.MaxSamplerIdx = m_EnvMap.EnvTexture()->GetSRDescriptor().Index();

	//mPerFrameConstants.ViewProjection = m_Scene.GetCamera().GetViewProjMat();
	//mPerFrameConstants.Position = m_Scene.GetCamera().GetPos();
	mPerFrameConstants.camera = m_Scene.GetCamera().GetGPUStruct();

}

void App::Render()
{
	/* PRE RENDER */
	auto device = g_Engine->GetRenderBackend()->GetRenderDevice();



	device->SetGlobalRootConstant(mGlobalConstants);
	device->SetFrameRootConstants(mPerFrameConstants);




	/* Render */
	GraphicsContext& gfxContext = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(CommandType::Graphics).GetGraphicsContext();
	gfxContext.SetScene(&m_Scene);
	const FrameBuffer& frameBuffer = mFrameBuffers[m_Engine->GetRenderBackend()->GetCurrIndex()];

	gfxContext.TransitionResource(*frameBuffer.ColorBuffer, ResourceState::RenderTarget, true);
	gfxContext.SetRenderTargets(1, &frameBuffer.ColorBuffer->GetRTVDescriptor(0).CPUHandle(), frameBuffer.DepthBuffer->GetDSVDescriptor().CPUHandle());
	gfxContext.ClearColor(*frameBuffer.ColorBuffer);
	gfxContext.ClearDepth(*frameBuffer.DepthBuffer, D3D12_CLEAR_FLAG_DEPTH, 1.0, 0);
	gfxContext.SetViewportScissorRect(m_Viewport, m_Scissor);

	{
		SkyboxConstants instance{
			m_Scene.GetCamera().GetViewRotMat() * m_Scene.GetCamera().GetProjMat(),
			m_EnvMap.EnvTexture()->GetSRDescriptor().Index()
		};
		m_SkyboxPass.WriteData(instance);
		m_SkyboxPass.Execute(gfxContext, *m_Engine->GetRenderBackend()->GetResourceStorage(), &instance, sizeof(SkyboxConstants));
	}

	{
		GBufferPassConstants instance{ 
			m_EnvMap.EnvTexture()->GetSRDescriptor().Index(),
			m_EnvMap.IrradianceMap()->GetSRDescriptor().Index(),
			m_EnvMap.SpecBRDF()->GetSRDescriptor().Index()
		};
		m_GBufferPass.WriteData(instance);
		m_GBufferPass.Execute(gfxContext, *m_Engine->GetRenderBackend()->GetResourceStorage(), &instance, sizeof(GBufferPassConstants));
	}

	Texture* backBuffer = g_Engine->GetRenderBackend()->GetCurrBackBuffer();


	gfxContext.TransitionResource(*frameBuffer.ColorBuffer, ResourceState::PixelAccess);
	gfxContext.TransitionResource(*backBuffer, ResourceState::RenderTarget, true);
	gfxContext.SetPipelineState("Tonemap");
	gfxContext.SetRenderTargets(1, &backBuffer->GetRTVDescriptor(0).CPUHandle());
	Uint32 index = frameBuffer.ColorBuffer->GetSRDescriptor().Index();
	gfxContext.SetConstants(0, 1, &index);
	gfxContext.DrawInstanced(3, 1, 0, 0);


	/* Post Render */
	gfxContext.TransitionResource(*backBuffer, ResourceState::Present);
	gfxContext.Submit(true);

	g_Engine->GetRenderBackend()->Present();
}

bool App::Shutdown()
{
	ApplicationEntry::Shutdown();
	return false;
}

bool App::OnEvent(Kairos::Event& e)
{
	ApplicationEntry::OnEvent(e);
	m_Scene.OnEvent(e);
	return true;
}

void App::SetupResources() {

	auto backend = g_Engine->GetRenderBackend();
	auto device = backend->GetRenderDevice();
	{

		D3D12_SAMPLER_DESC linearSampler = {};
		linearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		linearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		linearSampler.MaxAnisotropy = 16;
		linearSampler.MaxLOD = D3D12_FLOAT32_MAX;
		linearSampler.MinLOD = 0.0f;
		linearSampler.MipLODBias = 0.0f;
		linearSamplerHandle = backend->GetRenderDevice()->CreateSampler(linearSampler);

		D3D12_SAMPLER_DESC aniSampler = {};
		aniSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aniSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aniSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		aniSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		aniSampler.Filter = D3D12_FILTER_ANISOTROPIC;
		aniSampler.MaxAnisotropy = 15;
		aniSampler.MaxLOD = D3D12_FLOAT32_MAX;
		aniSampler.MinLOD = 0.0f;
		aniSampler.MipLODBias = 0.0f;
		aniSamplerHandler = backend->GetRenderDevice()->CreateSampler(aniSampler);
	}


	projectDir = Filesystem::GetRelativePath("C:/Users/Chris Ting/Desktop/MeshRenderer/Data/");
	KRS_CORE_INFO(projectDir);

	float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
	std::wstring filePath = L"Data/shaders/";


	m_Scissor = CD3DX12_RECT(0, 0, 1024, 1024);
	m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, 1024, 1024, 0.f, 1.f);



	{
		mGlobalConstants.LinearClampSamplerIdx = linearSamplerHandle.Index();
		mGlobalConstants.AnisotropicClampSamplerIdx = aniSamplerHandler.Index();
		//mGlobalConstants.MaxSamplerIdx = m_EnvMap.EnvTexture()->GetSRDescriptor().Index();
		backend->GetRenderDevice()->SetGlobalRootConstant(mGlobalConstants);
		backend->GetRenderDevice()->SetFrameRootConstants(mPerFrameConstants);
	}

	m_Scene.Init(backend->GetRenderDevice());


	m_Scene.AddMesh(Mesh::LoadFromAsset(backend->GetRenderDevice(), "Data/assets_export/cerberus.mesh"));
	//m_Scene.AddMesh(Mesh::LoadFromAsset(device, "Data/assets_export/skybox.mesh"));
	m_Scene.AddMaterial(Material::LoadMatFromFolder(backend->GetRenderDevice(), "Data/textures/assets_export/", "cerberus"));
	m_Scene.AddLight(Light{ Vector4::One, Vector3(-1, 0, 0) });
	m_Scene.AddLight(Light{ Vector4::One, Vector3(1, 0, 0) });
	m_Scene.AddLight(Light{ Vector4::One, Vector3(0, -1 , 0) });


	
	m_TonemapPass.Setup(device, backend->PipelineManager());
	m_GBufferPass.Setup(device, backend->PipelineManager());
	m_SkyboxPass.Setup(device, backend->PipelineManager());

	m_EnvMap.Init(device, backend->PipelineManager());
	//

	//m_SkyboxBuffer = reinterpret_cast<DynamicBuffer*>(device->CreateDynConstBuffer(sizeof(SkyboxPass), sizeof(SkyboxPass)));


	backend->PipelineManager()->CompileAll();





	m_Scene.GPUStorage()->UploadMeshes();
	m_Scene.GPUStorage()->UploadMaterials();
	m_Scene.GPUStorage()->UploadLights();
}