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







bool App::Initialize()
{
	ApplicationEntry::Initialize();

	SetupResources();


	return true;
	
}

void App::Update()
{
	mCamera.Update(0.01f);
	m_CameraData.viewProjMat = mCamera.GetViewProjMat();
	m_CameraData.cameraPos = mCamera.GetPos();
	cbSkybox.viewRotProjMat = mCamera.GetViewRotMat() * mCamera.GetProjMat();

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

	GraphicsContext& gfxContext = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT).GetGraphicsContext();

	gfxContext.PIXBeginEvent(L"WHAT");
	auto& currBackBuffer = g_Engine->GetRenderBackend()->GetCurrBackBuffer();

	auto& frameBuffer = mFrameBuffers[g_Engine->GetRenderBackend()->CurrBufferIndex()];
	gfxContext.TransitionResource((*frameBuffer.ColorBuffer), D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	gfxContext.SetViewportScissorRect(g_Engine->GetRenderBackend()->GetViewport(), g_Engine->GetRenderBackend()->GetScissor());


	D3D12_CPU_DESCRIPTOR_HANDLE frameBufferRTV[] = {
		frameBuffer.ColorBuffer->GetRTV()
	};
	gfxContext.SetRenderTargets(_countof(frameBufferRTV), frameBufferRTV, frameBuffer.DepthBuffer->GetDSV());
	gfxContext.ClearColor(*frameBuffer.ColorBuffer.get());
	gfxContext.ClearDepth((*frameBuffer.DepthBuffer), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

	gfxContext.SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// skybox rendering
	{
		auto pso = g_Engine->GetRenderBackend()->GetRootSignature("Skybox").get();

		gfxContext.SetPipelineState(*(g_Engine->GetRenderBackend()->GetPipeline("Skybox").get()));
		gfxContext.SetRootSignature(*(g_Engine->GetRenderBackend()->GetRootSignature("Skybox").get()));
		gfxContext.SetVertexBuffers(0, 1, mSkyboxMesh->GetVertexView());
		gfxContext.SetIndexBuffer(mSkyboxMesh->GetIndexView());
		gfxContext.SetDynamicCBV(0, sizeof(cbSkybox), &cbSkybox);
		gfxContext.SetDynamicDescriptors(1, 0, 1, m_envTexture->GetSRV());
		gfxContext.DrawIndexedInstance(mSkyboxMesh->mIndices.size(), 1, 0, 0, 0);
	}

	{
		TransformCB transformCBV{ cube1WorldMat, mCamera.GetViewProjMat() };
		ShadingCB shadingConstants;
		shadingConstants.eyePosition = mCamera.GetPos();
		for (int i = 0; i < SceneSettings::NumLights; i++) {
			const SceneSettings::Light& light = g_SceneSettings.lights[i];
			shadingConstants.lights[i].direction = light.direction;
			shadingConstants.lights[i].radiance = light.radiance;
		}

		gfxContext.SetPipelineState(*(g_Engine->GetRenderBackend()->GetPipeline("PBR")));
		gfxContext.SetRootSignature(*(g_Engine->GetRenderBackend()->GetRootSignature("PBR")));

		mMainMesh->Bind(gfxContext);

		gfxContext.SetDynamicCBV(0, sizeof(TransformCB), &transformCBV);
		gfxContext.SetDynamicCBV(1, sizeof(ShadingCB), &shadingConstants);
		gfxContext.SetDynamicDescriptors(2, 0, 4, mTexture->GetSRV());
		gfxContext.SetDynamicDescriptors(2, 4, 1, m_envTexture->GetSRV());
		gfxContext.SetDynamicDescriptors(2, 5, 1, m_irmapTexture->GetSRV());
		gfxContext.SetDynamicDescriptors(2, 6, 1, m_spBRDF_LUT->GetSRV());

		mMainMesh->Draw(gfxContext);
	}

	gfxContext.PIXSetMarker(L"Test Label");

	gfxContext.TransitionResource(currBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] =
	{
		currBackBuffer.GetRTV()
	};
	gfxContext.SetRenderTargets(_countof(RTVs), RTVs);
	gfxContext.ClearColor(currBackBuffer);

	{
		gfxContext.SetRootSignature(*(g_Engine->GetRenderBackend()->GetRootSignature("Tonemap").get()));
		gfxContext.SetPipelineState(*(g_Engine->GetRenderBackend()->GetPipeline("Tonemap").get()));
		gfxContext.SetDynamicDescriptors(0, 0, 1, frameBuffer.ColorBuffer->GetSRV());
		gfxContext.DrawIndexedInstance(3, 1, 0, 0, 0);
	}
	gfxContext.PIXEndEvent();

	g_Engine->GetGUISystem()->Render(gfxContext);


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
	mCamera.OnEvent(e);

	return true;
}

void App::SetupResources() {

	Vector3 one = Vector3::Left;
	Vector3 two = Vector3::Right;
	Vector3 three = Vector3::Down;

	one.Normalize();
	two.Normalize();
	three.Normalize();

	g_SceneSettings.lights[0].direction = one;
	g_SceneSettings.lights[1].direction = two;
	g_SceneSettings.lights[2].direction = three;

	g_SceneSettings.lights[0].radiance = Vector3::One;
	g_SceneSettings.lights[1].radiance = Vector3::One;
	g_SceneSettings.lights[2].radiance = Vector3::One;

	projectDir = Filesystem::GetRelativePath("C:/Users/Chris Ting/Desktop/MeshRenderer/Data/");
	KRS_CORE_INFO(projectDir);
	LoadPBRTextures();
	CreateMeshes();



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

	{
		Ref<RootSignature> TonemapSig = backend->AllocateRootSignature("Tonemap", 1, 1);
		Ref<GraphicsPSO> TonemapPSO = backend->AllocateGraphicsPSO("Tonemap");

		(*TonemapSig)[0].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*TonemapSig)[0].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		TonemapSig->SetStaticSampler(0, computeSamplerDesc);
		TonemapSig->Finalize(L"Tonemap Root Signature",
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS);

		TonemapVS = CreateScope<Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
			ShaderCreateInfo(ShaderType::Vertex, filePath + L"tonemap.hlsl", "main_vs"));
		TonemapPS = CreateScope<Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
			ShaderCreateInfo(ShaderType::Pixel, filePath + L"tonemap.hlsl", "main_ps"));

		TonemapPSO->SetRootSignature(*TonemapSig);
		TonemapPSO->SetVertexShader(TonemapVS);
		TonemapPSO->SetPixelShader(TonemapPS);
		TonemapPSO->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		TonemapPSO->SetDepthStencilState(D3D12_DEPTH_STENCIL_DESC{});
		TonemapPSO->SetRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);

		if (TonemapPSO == nullptr) KRS_CORE_ERROR("FIJEWIPFJEWP");
		TonemapPSO->Finalize();
	}



	// PBR Pipeline 


	{
		Ref<RootSignature> PBRSignature = backend->AllocateRootSignature("PBR", 3, 2);
		Ref<GraphicsPSO> pbrPSO = backend->AllocateGraphicsPSO("PBR");

		(*PBRSignature)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		(*PBRSignature)[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
		(*PBRSignature)[2].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*PBRSignature)[2].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0, 0);
		PBRSignature->SetStaticSampler(0, defaultSamplerDesc);
		PBRSignature->SetStaticSampler(1, spBRDF_SamplerDesc);
		PBRSignature->Finalize(L"PBR Root Signature", rootSignatureFlags);

		ShaderCreateInfo Vinfo(ShaderType::Vertex, filePath + L"pbr.hlsl", "main_vs");
		Ref<Shader> VS = CreateRef<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(), Vinfo);

		ShaderCreateInfo PInfo(ShaderType::Pixel, filePath + L"pbr.hlsl", "main_ps");
		Ref<Shader> PS = CreateRef<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(), PInfo);

		pbrPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ PBRLayout, _countof(PBRLayout) });
		pbrPSO->SetRootSignature(*PBRSignature);
		pbrPSO->SetVertexShader(VS);
		pbrPSO->SetPixelShader(PS);
		pbrPSO->SetRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
		pbrPSO->Finalize();
	}




	{
		// skybox PSO
		{
			Ref<RootSignature> SkyboxRS = backend->AllocateRootSignature("Skybox", 2, 1);
			Ref<GraphicsPSO> SkyboxPSO = backend->AllocateGraphicsPSO("Skybox");

			Ref<Kairos::Shader> skyboxVS = std::make_unique<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
				ShaderCreateInfo(ShaderType::Vertex, filePath + L"skybox.hlsl", "main_vs"));
			Ref<Kairos::Shader> skyboxPS = std::make_unique<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
				ShaderCreateInfo(ShaderType::Pixel, filePath + L"skybox.hlsl", "main_ps"));

			(*SkyboxRS)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX, 0);
			(*SkyboxRS)[1].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
			(*SkyboxRS)[1].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

			SkyboxRS->SetStaticSampler(0, defaultSamplerDesc);
			SkyboxRS->Finalize(L"Skybox Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


			SkyboxPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ skyboxLayout, _countof(skyboxLayout) });
			SkyboxPSO->SetRootSignature(*SkyboxRS);
			SkyboxPSO->SetVertexShader(skyboxVS);
			SkyboxPSO->SetPixelShader(skyboxPS);

			SkyboxPSO->SetRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
			SkyboxPSO->InitializeCommonFeatures();
			D3D12_DEPTH_STENCIL_DESC depthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			SkyboxPSO->SetDepthStencilState(depthDesc);
			SkyboxPSO->Finalize();
		}


		Ref<RootSignature> computeRootSig = backend->AllocateRootSignature("Compute", 3, 1);
		(*computeRootSig)[0].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_ALL);
		(*computeRootSig)[0].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		(*computeRootSig)[1].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_ALL);
		(*computeRootSig)[1].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
		(*computeRootSig)[2].InitAsConstants(0, 1, D3D12_SHADER_VISIBILITY_ALL);
		computeRootSig->SetStaticSampler(0, computeSamplerDesc);
		computeRootSig->Finalize(L"Compute Root Sig",
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);


		m_envTexture = g_Engine->GetRenderBackend()->CreateTexture(1024, 1024, 6, 
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		// creates cube texture SRV & UAV
		{

			envTextureUnfilter = g_Engine->GetRenderBackend()->CreateTexture(1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


			// creates another cubeTexture SRV & 0th level MIP UAV;
			//UAV is where the equirectangular to cubemap result will be stored

			skyboxTexture = Texture::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(),
				(projectDir + "textures/assets_export/environment.tx").c_str(), DXGI_FORMAT_R32G32B32A32_FLOAT);
			// equirecntangular -> cubemap
			{
				

				Ref<Kairos::Shader> compute = std::make_shared<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"equirect2cube.hlsl", "main"));


				Ref<ComputePSO> computePSO = backend->AllocateComputePSO("Equi2Cube");
				computePSO->SetRootSignature(*computeRootSig);
				computePSO->SetComputeShader(compute);
				computePSO->Finalize();

				Kairos::ComputeContext& context = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
				context.SetPipelineState(*computePSO);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(0, 0, 1, skyboxTexture->GetSRV());
				context.SetDynamicDescriptors(1, 0, 1, envTextureUnfilter->GetUAV(0));
				context.Dispatch(1024 / 32, 1024 / 32, 6);
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_COMMON, true);

				
				context.Submit(true);
			}



			// specular pre filter environment
			{
				Ref<Kairos::Shader> compute = CreateRef<Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"spmap.hlsl", "main"));
				Ref<ComputePSO> computePSO = backend->AllocateComputePSO("SpecularEnv");
				computePSO->SetRootSignature(*computeRootSig);
				computePSO->SetComputeShader(compute);
				computePSO->Finalize();

				ComputeContext& context = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*m_envTexture, D3D12_RESOURCE_STATE_COPY_DEST);
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_COPY_SOURCE);

				for (Uint32 arraySlice = 0; arraySlice < 6; ++arraySlice) {
					const UINT subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, m_envTexture->GetDesc().MipLevels, 6);
					context.CopySubresource(*m_envTexture, subresourceIndex, *envTextureUnfilter, subresourceIndex);
				}
				context.TransitionResource(*m_envTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


				context.SetPipelineState(*computePSO);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(0, 0, 1, envTextureUnfilter->GetSRV());

				const float deltaRoughness = 1.0f / Math::Max<float>(float(m_envTexture->MipLevels() - 1), 1.0f);
				for (UINT level = 0, size = 512; level < m_envTexture->MipLevels(); ++level, size /= 2) {
					const UINT numGroups = std::max<UINT>(1, size / 32);
					const float spmapRoughness = level * deltaRoughness;

					context.SetDynamicDescriptors(1, 0, 1, m_envTexture->GetUAV(level));
					context.SetConstantArray(2, 1, &spmapRoughness);
					context.Dispatch(numGroups, numGroups, 6);
				}

				context.Submit(true);
			}


			m_irmapTexture = g_Engine->GetRenderBackend()->CreateTexture(256, 256, 6, DXGI_FORMAT_R16G16B16A16_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			{
				Ref<Kairos::Shader> compute = std::make_shared<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"irmap.hlsl", "main"));


				Ref<ComputePSO> computePSO = backend->AllocateComputePSO("Shit");
				computePSO->SetRootSignature(*computeRootSig);
				computePSO->SetComputeShader(compute);
				computePSO->Finalize();

				Kairos::ComputeContext& context = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*m_irmapTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
				context.SetPipelineState(*computePSO);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(0, 0, 1, envTextureUnfilter->GetSRV());
				context.SetDynamicDescriptors(1, 0, 1, m_irmapTexture->GetUAV(0));
				context.Dispatch(m_irmapTexture->Width() / 32, m_irmapTexture->Height() / 32, 6);
				context.TransitionResource(*m_irmapTexture, D3D12_RESOURCE_STATE_COMMON, true);


				context.Submit(true);
			}

			// 1 mip level 1 Texture2D
			m_spBRDF_LUT = g_Engine->GetRenderBackend()->CreateTexture(256, 256, 1, DXGI_FORMAT_R16G16_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 1);
			{
				Ref<Kairos::Shader> compute = std::make_shared<Kairos::Shader>(g_Engine->GetRenderBackend()->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"spbrdf.hlsl", "main"));

				Ref<ComputePSO> computePSO = backend->AllocateComputePSO("CookLUT");
				computePSO->SetRootSignature(*computeRootSig);
				computePSO->SetComputeShader(compute);
				computePSO->Finalize();

				ComputeContext& context = g_Engine->GetRenderBackend()->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*m_spBRDF_LUT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
				context.SetPipelineState(*computePSO);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(1, 0, 1, m_spBRDF_LUT->GetUAV(0));
				context.Dispatch(m_spBRDF_LUT->Width() / 32, m_spBRDF_LUT->Height() / 32, 1);

				context.TransitionResource(*m_spBRDF_LUT, D3D12_RESOURCE_STATE_COMMON);
				context.Submit(true);
			}


		}

	}


	{

		for (int i = 0; i < 2; i++) {
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Width = 1024;
			desc.Height = 1024;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			const float color[] = { 0.25f, 0.25f, 0.25f, 1.f };

			D3D12_CLEAR_VALUE clearVal = CD3DX12_CLEAR_VALUE{ desc.Format, color };

			mFrameBuffers[i].ColorBuffer = CreateRef<Texture>(g_Engine->GetRenderBackend()->GetRenderDevice(), desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal);

			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			clearVal = CD3DX12_CLEAR_VALUE{ desc.Format, 1.0f, 0 };

			mFrameBuffers[i].DepthBuffer = CreateRef<Texture>(g_Engine->GetRenderBackend()->GetRenderDevice(), desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal);
		}
	}



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

void App::LoadPBRTextures()
{
	mTexture = Texture::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(), "Data/textures/assets_export/cerberus_A.tx", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	mNormalMap = Texture::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(), "Data/textures/assets_export/cerberus_N.tx", DXGI_FORMAT_R8G8B8A8_UNORM);
	mMetalTex = Texture::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(), "Data/textures/assets_export/cerberus_M.tx", DXGI_FORMAT_R8_UNORM);
	mRoughnessMap = Texture::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(), "Data/textures/assets_export/cerberus_R.tx", DXGI_FORMAT_R8_UNORM);
}

void App::CreateMeshes()
{
	mMesh = Mesh::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(),
		"Data/assets_export/Cube.mesh");
	mSkyboxMesh = Mesh::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(),
		"Data/assets_export/skybox.mesh");
	mMainMesh = Mesh::LoadFromAsset(g_Engine->GetRenderBackend()->GetRenderDevice(),
		"Data/assets_export/Cerberus.mesh");
}
