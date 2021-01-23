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
	mPerFrameConstants.ViewProjection = m_Scene.GetCamera().GetViewProjMat();
	mPerFrameConstants.Position = m_Scene.GetCamera().GetPos();
}

void App::Render()
{
	/* PRE RENDER */
	auto* device = g_Engine->GetRenderBackend()->GetRenderDevice();

	mGlobalConstants.LinearClampSamplerIdx = linearSamplerHandle.Index();
	device->SetGlobalRootConstant(mGlobalConstants);
	device->SetFrameRootConstants(mPerFrameConstants);




	/* Render */
	GraphicsContext& gfxContext = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(CommandType::Graphics).GetGraphicsContext();
	const FrameBuffer& frameBuffer = mFrameBuffers[m_Engine->GetRenderBackend()->GetCurrIndex()];

	gfxContext.TransitionResource(*frameBuffer.ColorBuffer, ResourceState::RenderTarget, true);
	gfxContext.SetRenderTargets(1, &frameBuffer.ColorBuffer->GetRTVDescriptor(0).CPUHandle(), frameBuffer.DepthBuffer->GetDSVDescriptor().CPUHandle());
	gfxContext.ClearColor(*frameBuffer.ColorBuffer);
	gfxContext.ClearDepth(*frameBuffer.DepthBuffer, D3D12_CLEAR_FLAG_DEPTH, 1.0, 0);



	gfxContext.SetPipelineState("PBR");
	gfxContext.SetViewportScissorRect(m_Viewport, m_Scissor);



	m_Scene.Bind(gfxContext);
	gfxContext.DrawInstanced(m_Scene.MergedMeshes()[0].Indices().size(), 1, 0, 0);

	gfxContext.SetPipelineState("Skybox");
	m_Scene.Bind(gfxContext);
	SkyboxPass instance{
		m_Scene.GetCamera().GetViewRotMat(),
		m_Scene.MergedMeshes().back().TempLocation.firstIndex,
		m_Scene.MergedMeshes().back().TempLocation.firstVert,
		4
	};
	memcpy(m_SkyboxBuffer->DataPtr(), &instance, sizeof(SkyboxPass));
	gfxContext.BindPassConstantBuffer(m_SkyboxBuffer->GPUVirtualAdress());
	gfxContext.DrawInstanced(m_Scene.MergedMeshes().back().Indices().size(), 1, 0, 0);

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




	projectDir = Filesystem::GetRelativePath("C:/Users/Chris Ting/Desktop/MeshRenderer/Data/");
	KRS_CORE_INFO(projectDir);

	float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
	std::wstring filePath = L"Data/shaders/";


	m_Scissor = CD3DX12_RECT(0, 0, 1024, 1024);
	m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, 1024, 1024, 0.f, 1.f);

	auto backend = g_Engine->GetRenderBackend();
	auto device = backend->GetRenderDevice();
	backend->GetRenderDevice()->SetGlobalRootConstant(mGlobalConstants);
	backend->GetRenderDevice()->SetFrameRootConstants(mPerFrameConstants);
	m_Scene.Init(backend->GetRenderDevice());


	m_Scene.AddMesh(Mesh::LoadFromAsset(backend->GetRenderDevice(), "Data/assets_export/cerberus.mesh"));
	m_Scene.AddMesh(Mesh::LoadFromAsset(device, "Data/assets_export/skybox.mesh"));
	m_Scene.AddMaterial(Material::LoadMatFromFolder(backend->GetRenderDevice(), "Data/textures/assets_export/", "cerberus"));
	m_Scene.AddLight(Light{ Vector4::One, Vector3(-1, 0, 0) });
	m_Scene.AddLight(Light{ Vector4::One, Vector3(1, 0, 0) });
	m_Scene.AddLight(Light{ Vector4::One, Vector3(0, -1 , 0) });


	// Tonemap Render Pass
	backend->GetRenderDevice()->CreateRootSignature("Tonemap", [](RootSignatureProxy& sig)
		{
			sig.AddRootConstant(std::ceil(sizeof(Uint32) / 4.f), 0, 0);
		});

	backend->GetRenderDevice()->CreateGraphicsPSO("Tonemap", [&](GraphicsPipelineProxy& proxy)
		{
			proxy.VSFile = "tonemap.hlsl";
			proxy.PSFile = "tonemap.hlsl";
			proxy.RootSigName = "Tonemap";
			proxy.DepthStencilState.DepthEnable = false;
			proxy.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
			proxy.RenderTargetFormats = {
				DXGI_FORMAT_R8G8B8A8_UNORM,
			};
		});


	// PBR Render Pass
	backend->GetRenderDevice()->CreateRootSignature("Base", [](RootSignatureProxy& sig)
		{
			//sig.AddRootConstant(std::ceil(sizeof(Uint32) / 4.f), 0, 0);
			sig.AddSRParam(0, 0);
			sig.AddSRParam(1, 0);
			sig.AddSRParam(2, 0);
			sig.AddSRParam(3, 0);
		});

	backend->GetRenderDevice()->CreateGraphicsPSO("PBR", [&](GraphicsPipelineProxy& proxy)
		{
			proxy.VSFile = "test.hlsl";
			proxy.PSFile = "test.hlsl";
			proxy.RootSigName = "Base";
			proxy.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			proxy.RenderTargetFormats = {
				DXGI_FORMAT_R16G16B16A16_FLOAT,
			};
			//proxy.InputLayout = D3D12_INPUT_LAYOUT_DESC{ PBRLayout, _countof(PBRLayout) };
		});





	device->CreateRootSignature("Skybox", [](RootSignatureProxy& sig)
		{
			sig.AddSRParam(0, 0);
			sig.AddSRParam(1, 0);
		});


	device->CreateGraphicsPSO("Skybox", [](GraphicsPipelineProxy& proxy)
		{
			proxy.VSFile = "skybox.hlsl";
			proxy.PSFile = "skybox.hlsl";
			proxy.RootSigName = "Skybox";
			proxy.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			proxy.RenderTargetFormats = {
				DXGI_FORMAT_R16G16B16A16_FLOAT,
			};
			proxy.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			proxy.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		});


	// EquirectToCube
	backend->GetRenderDevice()->CreateRootSignature("Default", [](RootSignatureProxy& proxy){});

	backend->GetRenderDevice()->CreateComputePSO("Equirect2Cube", [](ComputePiplineProxy& proxy)
		{
			proxy.CSFile = "equirect2cube.hlsl";
			proxy.RootSignatureName = "Default";
		}, true);

	envTexture = device->CreateTexture(TextureProperties(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, TextureType::Tex2D, 6), ResourceState::UnorderedAccess);

	unfilteredEnv = device->CreateTexture(TextureProperties(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, TextureType::Tex2D, 6), ResourceState::UnorderedAccess);
	Texture* equirectTex = device->CreateTexture("Data/textures/assets_export/environment.tx", DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1);

	device->TransientCommand([&](CommandContext& ctx)
		{
			struct EquiStruct {
				Uint32 inputTextureIndex;
				Uint32 outputTextureIndex;
			};
			EquiStruct instance{ equirectTex->GetUAVDescriptor(0).Index(), unfilteredEnv->GetSRDescriptor().Index() };

			DynamicBuffer* constants = reinterpret_cast<DynamicBuffer*>(device->CreateDynConstBuffer(sizeof(EquiStruct), sizeof(EquiStruct)));
			memcpy(constants->DataPtr(), &instance, sizeof(instance));

			auto& computeCtx = ctx.GetComputeContext();
			computeCtx.SetPipelineState("Equirect2Cube");
			computeCtx.BindPassConstantBuffer(constants->GPUVirtualAdress());
			computeCtx.Dispatch(envTexture->Width() / 32, envTexture->Height() / 32, 6);
		}, CommandType::Compute);
	

	m_SkyboxBuffer = reinterpret_cast<DynamicBuffer*>(device->CreateDynConstBuffer(sizeof(SkyboxPass), sizeof(SkyboxPass)));


	backend->GetRenderDevice()->CompileAll();



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
	


	m_Scene.UploadMeshes();
	m_Scene.UploadMaterials();
	m_Scene.UploadLights();
}