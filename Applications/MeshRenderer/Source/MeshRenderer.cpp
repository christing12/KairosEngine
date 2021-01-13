#include "apppch.h"
#include "MeshRenderer.h"

using namespace Kairos;


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




static D3D12_INPUT_ELEMENT_DESC PBRLayout[] = {
	{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

void MeshRenderer::DoSomething()
{
}

void MeshRenderer::OnEvent(Kairos::Event& e)
{
	Application::OnEvent(e);
	mCamera.OnEvent(e);
	EventDispatcher dispatcher(e);
}

void MeshRenderer::Update(float deltaTime)
{
	mCamera.Update(deltaTime);
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

void MeshRenderer::Shutdown()
{
	Application::Shutdown();
	mEditor.Shutdown();
}

void MeshRenderer::Render()
{
	

	GraphicsContext& gfxContext = mRenderer->GetGraphicsContext();
	gfxContext.PIXBeginEvent(L"WHAT");
	auto& currBackBuffer = mRenderer->GetCurrBackBuffer();

	auto& frameBuffer = mFrameBuffers[mRenderer->m_currBackBuffer];
	gfxContext.TransitionResource((*frameBuffer.ColorBuffer), D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	gfxContext.SetViewportScissorRect(mRenderer->m_Viewport, mRenderer->m_Scissor);


	D3D12_CPU_DESCRIPTOR_HANDLE frameBufferRTV[] = {
		frameBuffer.ColorBuffer->GetRTV()
	};
	gfxContext.SetRenderTargets(_countof(frameBufferRTV), frameBufferRTV, frameBuffer.DepthBuffer->GetDSV());
	gfxContext.ClearColor(*frameBuffer.ColorBuffer.get());
	gfxContext.ClearDepth((*frameBuffer.DepthBuffer), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

	gfxContext.SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//mRenderer->RenderTexture(gfxContext, *m_spBRDF_LUT);

	//// skybox rendering
	{
		
		gfxContext.SetPipelineState(*(mSkyboxPSO.get()));
		gfxContext.SetRootSignature(*(mSkyboxSignature.get()));
		gfxContext.SetVertexBuffers(0, 1, mSkyboxMesh->GetVertexView());
		gfxContext.SetIndexBuffer(mSkyboxMesh->GetIndexView());
		gfxContext.SetDynamicCBV(0, sizeof(cbSkybox), &cbSkybox);
		gfxContext.SetDynamicDescriptors(1, 0, 1, m_envTexture->GetSRV());
		gfxContext.DrawIndexedInstance(mSkyboxMesh->mIndices.size(), 1, 0, 0, 0);
	}

	//{
	//	gfxContext.SetPipelineState(*(mRenderer->DebugPSO.get()));
	//	gfxContext.SetRootSignature(*(mRenderer->DebugRS.get()));

	//	mMesh->Bind(gfxContext);

	//	cbPerObject.modelMat = Matrix::CreateScale(0.5f) * Matrix::CreateTranslation(light.position);
	//	cbPerObject.viewProjMat = mCamera.GetViewProjMat();

	//	gfxContext.SetDynamicCBV(0, sizeof(cbPerObject), &cbPerObject);
	//	
	//	mMesh->Draw(gfxContext);
	//	
	//}

	{
		TransformCB transformCBV{ cube1WorldMat, mCamera.GetViewProjMat() };
		ShadingCB shadingConstants;
		shadingConstants.eyePosition = mCamera.GetPos();
		for (int i = 0; i < SceneSettings::NumLights; i++) {
			const SceneSettings::Light& light = g_SceneSettings.lights[i];
			shadingConstants.lights[i].direction = light.direction;
			shadingConstants.lights[i].radiance = light.radiance;
		}

		gfxContext.SetPipelineState(*(mPSO.get()));
		gfxContext.SetRootSignature(*(mSig.get()));

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
		gfxContext.SetRootSignature(*(mTonemapSig.get()));
		gfxContext.SetPipelineState(*(mTonemapPSO.get()));
		gfxContext.SetDynamicDescriptors(0, 0, 1, frameBuffer.ColorBuffer->GetSRV());
		gfxContext.DrawIndexedInstance(3, 1, 0, 0, 0);
	}

	mEditor.Render(gfxContext);
	gfxContext.PIXEndEvent();

	gfxContext.Submit(true);
}

void MeshRenderer::Present()
{
	mRenderer->Present();
}

void MeshRenderer::LoadPBRTextures()
{
	mTexture = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "Data/textures/assets_export/Cerberus_A.tx", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	mNormalMap = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "Data/textures/assets_export/Cerberus_N.tx", DXGI_FORMAT_R8G8B8A8_UNORM);
	mMetalTex = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "Data/textures/assets_export/Cerberus_M.tx", DXGI_FORMAT_R8_UNORM);
	mRoughnessMap = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "Data/textures/assets_export/Cerberus_R.tx", DXGI_FORMAT_R8_UNORM);
}


void MeshRenderer::CreateMeshes()
{
	mMesh = Mesh::CreateFromFile(mRenderer->m_Device.get(),
		"Data/meshes/cube.obj");
	mSkyboxMesh = Mesh::CreateFromFile(mRenderer->m_Device.get(),
		"Data/meshes/skybox.obj");
	mMainMesh = Mesh::CreateFromFile(mRenderer->m_Device.get(),
		"Data/meshes/cerberus.fbx");
}

void MeshRenderer::InitEngine()
{

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
	mEditor = EngineIMGUI(mRenderer->m_Device.get(), mWindow.GetNativeWindow());
	std::wstring filePath = L"Data/shaders/";

	const  D3D12_INPUT_ELEMENT_DESC skyboxLayout[] = {
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	auto* pDevice = mRenderer->GetDevice();

	CD3DX12_STATIC_SAMPLER_DESC defaultSamplerDesc{ 0, D3D12_FILTER_ANISOTROPIC };
	defaultSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };

	CD3DX12_STATIC_SAMPLER_DESC spBRDF_SamplerDesc{ 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	spBRDF_SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	spBRDF_SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	spBRDF_SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	{
		mTonemapSig = CreateRef<RootSignature>(mRenderer->GetRenderDevice(), 1, 1);
		
		(*mTonemapSig)[0].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mTonemapSig)[0].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		mTonemapSig->SetStaticSampler(0, computeSamplerDesc);
		mTonemapSig->Finalize(L"Tonemap Root Signature",
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS);

		mTonemapPSO = CreateRef<GraphicsPSO>(mRenderer->GetRenderDevice());

		TonemapVS = CreateScope<Shader>(mRenderer->GetRenderDevice(),
			ShaderCreateInfo(ShaderType::Vertex, filePath + L"tonemap.hlsl", "main_vs"));
		TonemapPS = CreateScope<Shader>(mRenderer->GetRenderDevice(),
			ShaderCreateInfo(ShaderType::Pixel, filePath + L"tonemap.hlsl", "main_ps"));

		mTonemapPSO->SetRootSignature(*mTonemapSig);
		mTonemapPSO->SetVertexShader(TonemapVS);
		mTonemapPSO->SetPixelShader(TonemapPS);
		mTonemapPSO->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		mTonemapPSO->SetRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);

		mTonemapPSO->InitializeCommonFeatures();
		mTonemapPSO->Finalize();
	}

	// PBR PSO

	{
		mSig = CreateRef<RootSignature>(mRenderer->m_Device.get(), 3, 2);

		(*mSig)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		(*mSig)[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mSig)[2].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mSig)[2].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0, 0);
		mSig->SetStaticSampler(0, defaultSamplerDesc);
		mSig->SetStaticSampler(1, spBRDF_SamplerDesc);
		mSig->Finalize(L"PBR Root Signature", rootSignatureFlags);


		ShaderCreateInfo Vinfo(ShaderType::Vertex, filePath + L"pbr.hlsl", "main_vs");
		mVertexShader = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), Vinfo);

		ShaderCreateInfo PInfo(ShaderType::Pixel, filePath + L"pbr.hlsl", "main_ps");
		mPixelShader = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), PInfo);

		mPSO = CreateRef<GraphicsPSO>(mRenderer->GetRenderDevice());

		ID3DBlob* blob = mVertexShader->GetD3DBlob();
		ID3DBlob* sBlob = mPixelShader->GetD3DBlob();

		mPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ PBRLayout, _countof(PBRLayout) });
		mPSO->SetRootSignature(*mSig);
		mPSO->SetVertexShader(mVertexShader);
		mPSO->SetPixelShader(mPixelShader);

		mPSO->SetRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);

		mPSO->Finalize();
	}

	{
		// skybox PSO
		{
			mSkyboxPSO = std::make_shared<GraphicsPSO>(mRenderer->GetRenderDevice());
			Ref<Kairos::Shader> skyboxVS = std::make_unique<Kairos::Shader>(mRenderer->GetRenderDevice(),
				ShaderCreateInfo(ShaderType::Vertex, filePath + L"skybox.hlsl", "main_vs"));
			Ref<Kairos::Shader> skyboxPS = std::make_unique<Kairos::Shader>(mRenderer->GetRenderDevice(),
				ShaderCreateInfo(ShaderType::Pixel, filePath + L"skybox.hlsl", "main_ps"));

			mSkyboxSignature = std::make_shared<Kairos::RootSignature>(mRenderer->m_Device.get(), 2, 1);
			(*mSkyboxSignature)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX, 0);
			(*mSkyboxSignature)[1].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
			(*mSkyboxSignature)[1].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

			mSkyboxSignature->SetStaticSampler(0, defaultSamplerDesc);
			mSkyboxSignature->Finalize(L"Skybox Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


			mSkyboxPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ skyboxLayout, _countof(skyboxLayout) });
			mSkyboxPSO->SetRootSignature(*mSkyboxSignature);
			mSkyboxPSO->SetVertexShader(skyboxVS);
			mSkyboxPSO->SetPixelShader(skyboxPS);

			mSkyboxPSO->SetRenderTarget(DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
			mSkyboxPSO->InitializeCommonFeatures();
			D3D12_DEPTH_STENCIL_DESC depthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			mSkyboxPSO->SetDepthStencilState(depthDesc);


			mSkyboxPSO->Finalize();
		}


		CD3DX12_ROOT_PARAMETER1;
		Ref<RootSignature> computeRootSig = CreateRef<RootSignature>(mRenderer->GetRenderDevice(), 3, 1);
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


		m_envTexture = mRenderer->CreateTexture(1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		// creates cube texture SRV & UAV
		{

			envTextureUnfilter = mRenderer->CreateTexture(1024, 1024, 6, DXGI_FORMAT_R16G16B16A16_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			// creates another cubeTexture SRV & 0th level MIP UAV;
			//UAV is where the equirectangular to cubemap result will be stored


			// equirecntangular -> cubemap
			{
				skyboxTexture = Texture::LoadFromAsset(mRenderer->GetRenderDevice(),
					(projectDir + "textures/assets_export/environment.tx").c_str(), DXGI_FORMAT_R32G32B32A32_FLOAT);

				Ref<Kairos::Shader> compute = std::make_shared<Kairos::Shader>(mRenderer->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"equirect2cube.hlsl", "main"));


				m_computePSO1 = CreateRef<ComputePSO>(mRenderer->GetRenderDevice());
				m_computePSO1->SetRootSignature(*computeRootSig);
				m_computePSO1->SetComputeShader(compute);
				m_computePSO1->Finalize();

				Kairos::ComputeContext& context = mRenderer->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
				context.SetPipelineState(*m_computePSO1);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(0, 0, 1, skyboxTexture->GetSRV());
				context.SetDynamicDescriptors(1, 0, 1, envTextureUnfilter->GetUAV(0));
				context.Dispatch(1024 / 32, 1024 / 32, 6);
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_COMMON, true);
				context.Submit(true);
			}



			{
				Ref<Kairos::Shader> compute = CreateRef<Shader>(mRenderer->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"spmap.hlsl", "main"));
				m_computePSO2 = CreateRef<ComputePSO>(mRenderer->GetRenderDevice());
				m_computePSO2->SetRootSignature(*computeRootSig);
				m_computePSO2->SetComputeShader(compute);
				m_computePSO2->Finalize();
			
				ComputeContext& context = mRenderer->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*m_envTexture, D3D12_RESOURCE_STATE_COPY_DEST);
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_COPY_SOURCE);

				for (Uint32 arraySlice = 0; arraySlice < 6; ++arraySlice) {
					const UINT subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, m_envTexture->GetDesc().MipLevels, 6);
					context.CopySubresource(*m_envTexture, subresourceIndex, *envTextureUnfilter, subresourceIndex);
				}
				context.TransitionResource(*m_envTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				context.TransitionResource(*envTextureUnfilter, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


				context.SetPipelineState(*m_computePSO2);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(0, 0, 1, envTextureUnfilter->GetSRV());

				const float deltaRoughness = 1.0f / Math::Max<float>(float(m_envTexture->MipLevels() - 1), 1.0f);
				for (UINT level = 1, size = 512; level < m_envTexture->MipLevels(); ++level, size /= 2) {
					const UINT numGroups = std::max<UINT>(1, size / 32);
					const float spmapRoughness = level * deltaRoughness;

					context.SetDynamicDescriptors(1, 0, 1, m_envTexture->GetUAV(level));
					context.SetConstantArray(2, 1, &spmapRoughness);
					context.Dispatch(numGroups, numGroups, 6);
				}

				context.Submit(true);
			}







			m_irmapTexture = mRenderer->CreateTexture(32, 32, 6, DXGI_FORMAT_R16G16B16A16_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 1);
			// creates SRV & UAV
			{
				Ref<Kairos::Shader> compute = std::make_shared<Kairos::Shader>(mRenderer->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"irmap.hlsl", "main"));

				m_computePSO3 = CreateRef<ComputePSO>(mRenderer->GetRenderDevice());
				m_computePSO3->SetRootSignature(*computeRootSig);
				m_computePSO3->SetComputeShader(compute);
				m_computePSO3->Finalize();

				Kairos::ComputeContext& context = mRenderer->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*m_irmapTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
				context.SetPipelineState(*m_computePSO3);
				context.SetRootSignature(*computeRootSig);
				context.SetDynamicDescriptors(0, 0, 1, m_envTexture->GetSRV());
				context.SetDynamicDescriptors(1, 0, 1, m_irmapTexture->GetUAV(0));
				context.Dispatch(m_irmapTexture->Width() / 32, m_irmapTexture->Height() / 32, 6);
				context.TransitionResource(*m_irmapTexture, D3D12_RESOURCE_STATE_COMMON, true);
				context.Submit(true);
				
			}

			m_spBRDF_LUT = mRenderer->CreateTexture(256, 256, 1, DXGI_FORMAT_R16G16B16A16_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 1);
			{
				Ref<Kairos::Shader> compute = std::make_shared<Kairos::Shader>(mRenderer->GetRenderDevice(),
					ShaderCreateInfo(ShaderType::Compute, filePath + L"spbrdf.hlsl", "main"));

				m_computePSO4 = CreateRef<ComputePSO>(mRenderer->GetRenderDevice());
				m_computePSO4->SetRootSignature(*computeRootSig);
				m_computePSO4->SetComputeShader(compute);
				m_computePSO4->Finalize();

				ComputeContext& context = mRenderer->GetRenderDevice()->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();
				context.TransitionResource(*m_spBRDF_LUT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
				context.SetPipelineState(*m_computePSO4);
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

			mFrameBuffers[i].ColorBuffer = CreateRef<Texture>(mRenderer->GetRenderDevice(), desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal);



			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			clearVal = CD3DX12_CLEAR_VALUE{ desc.Format, 1.0f, 0 };

			mFrameBuffers[i].DepthBuffer = CreateRef<Texture>(mRenderer->GetRenderDevice(), desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal);
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
