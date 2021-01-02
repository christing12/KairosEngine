#include "apppch.h"
#include "MeshRenderer.h"

using namespace Kairos;


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
		cube1WorldMat = worldMat;
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
	auto& currBackBuffer = mRenderer->GetCurrBackBuffer();

	gfxContext.TransitionResource(currBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] =
	{
		currBackBuffer.GetRTV()
	};
	gfxContext.SetRenderTargets(_countof(RTVs), RTVs, mRenderer->GetDSV());
	gfxContext.ClearColor(currBackBuffer);
	gfxContext.ClearDepth(mRenderer->GetDepthBuffer(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

	gfxContext.SetViewportScissorRect(mRenderer->m_Viewport, mRenderer->m_Scissor);
	gfxContext.SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	////// skybox rendering
	{
		
		gfxContext.SetPipelineState(*(mSkyboxPSO.get()));
		gfxContext.SetRootSignature(*(mSkyboxSignature.get()));
		gfxContext.SetVertexBuffers(0, 1, skyboxVView);
		gfxContext.SetIndexBuffer(skyboxIView);
		gfxContext.SetDynamicCBV(0, sizeof(cbSkybox), &cbSkybox);
		gfxContext.SetDynamicDescriptors(1, 0, 1, skyboxTexture->GetSRV());
		gfxContext.DrawIndexedInstance(mSkyboxMesh->mIndices.size(), 1, 0, 0, 0);
	}


	// model rendering
	{
		gfxContext.SetPipelineState(*(mPSO.get()));
		gfxContext.SetRootSignature(*(mSig.get()));

		gfxContext.SetVertexBuffers(0, 1, mVBView);
		gfxContext.SetIndexBuffer(mIBView);

		cbPerObject.modelMat = cube1WorldMat;
		cbPerObject.viewProjMat = mCamera.GetViewProjMat();

		gfxContext.SetDynamicCBV(0, sizeof(cbPerObject), &cbPerObject);
		gfxContext.SetDynamicCBV(1, sizeof(Light), &light);
		gfxContext.SetDynamicCBV(2, sizeof(CameraData), &m_CameraData);
		gfxContext.SetDynamicDescriptors(3, 0, 4, mTexture->GetSRV());
		//gfxContext.SetDynamicDescriptors(3, 1, 1, mNormalMap->GetSRV());
		gfxContext.DrawIndexedInstance(mMesh->mIndices.size(), 1, 0, 0, 0);
		//
		//cbPerObject.modelMat = Matrix::CreateTranslation(light.position);
		//gfxContext.SetDynamicCBV(0, sizeof(cbPerObject), &cbPerObject);
		//gfxContext.DrawIndexedInstance(mMesh->mIndices.size(), 1, 0, 0, 0);
	}


	mEditor.Render(gfxContext);

	gfxContext.Submit(true);
}

void MeshRenderer::Present()
{
	mRenderer->Present();
}

void MeshRenderer::LoadPBRTextures()
{
	mTexture = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "Cerberus_A.tx", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	mNormalMap = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "../../Data/textures/assets_export/Cerberus_N.tx");
	mMetalTex = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "../../Data/textures/assets_export/Cerberus_M.tx", DXGI_FORMAT_R8_UNORM);
	mRoughnessMap = Texture::LoadFromAsset(mRenderer->GetRenderDevice(), "../../Data/textures/assets_export/Cerberus_R.tx", DXGI_FORMAT_R8_UNORM);
}

void MeshRenderer::InitEngine()
{
	projectDir = Filesystem::GetRelativePath("C:/Users/Chris Ting/Desktop/MeshRenderer/Data/");
	KRS_CORE_INFO(projectDir);
	LoadPBRTextures();


	float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
	mEditor = EngineIMGUI(mRenderer->m_Device.get(), mWindow.GetNativeWindow());
	std::wstring filePath = L"../../KairosEngine/Engine/Graphics/Shaders/";

	const  D3D12_INPUT_ELEMENT_DESC skyboxLayout[] = {
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};


	D3D12_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_ANISOTROPIC;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MinLOD = 0.f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	auto* pDevice = mRenderer->GetDevice();

	//// debug stuff
	//{
	//	m_WhiteRS = CreateRef<RootSignature>(mRenderer->m_Device.get(), 0, 0);
	//	m_WhiteRS->Finalize(L"DEBUG ROOT SIGNATURE", rootSignatureFlags);

	//	ShaderCreateInfo Vinfo(ShaderType::Vertex, filePath + L"white.hlsl", "main");
	//	m_WhiteVS = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), Vinfo);

	//	ShaderCreateInfo PInfo(ShaderType::Pixel, filePath + L"white.hlsl", "main_ps");
	//	m_WhitePS = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), PInfo);

	//	m_WhitePSO = CreateRef<PipelineStateObject>(mRenderer->GetRenderDevice());

	//	ID3DBlob* blob = m_WhitePS->GetD3DBlob();
	//	ID3DBlob* sBlob = mPixelShader->GetD3DBlob();

	//	m_WhitePSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ skyboxLayout, _countof(skyboxLayout) });
	//	m_WhitePSO->SetRootSignature(*m_WhiteRS);
	//	m_WhitePSO->SetVertexShader(CD3DX12_SHADER_BYTECODE(blob));
	//	m_WhitePSO->SetPixelShader(CD3DX12_SHADER_BYTECODE(sBlob));
	//	m_WhitePSO->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	//	m_WhitePSO->SetRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
	//	m_WhitePSO->SetSampleMask(0xffffffff);

	//	D3D12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//	rasterDesc.FrontCounterClockwise = true;
	//	m_WhitePSO->SetRasterizerState(rasterDesc);
	//	m_WhitePSO->SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
	//	m_WhitePSO->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));

	//	m_WhitePSO->Finalize();
	//}


	{


		mSig = CreateRef<RootSignature>(mRenderer->m_Device.get(), 4, 1);

		(*mSig)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		(*mSig)[1].InitAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mSig)[2].InitAsConstantBuffer(2, D3D12_SHADER_VISIBILITY_PIXEL);

		(*mSig)[3].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mSig)[3].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0);

		mSig->InitStaticSampler(0, sampler, D3D12_SHADER_VISIBILITY_PIXEL);

		
		mSig->Finalize(L"ROOT SIGNATURE", rootSignatureFlags);
	}
	skyboxTexture = Texture::LoadFromAsset(mRenderer->GetRenderDevice(),
		(projectDir + "textures/assets_export/environment.tx").c_str(), DXGI_FORMAT_R32G32B32A32_FLOAT);



	{

		ShaderCreateInfo Vinfo(ShaderType::Vertex, filePath + L"phong.hlsl", "main_vs");
		mVertexShader = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), Vinfo);

		ShaderCreateInfo PInfo(ShaderType::Pixel, filePath + L"phong.hlsl", "main_ps");
		mPixelShader = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), PInfo);

		mPSO = CreateRef<PipelineStateObject>(mRenderer->GetRenderDevice());

		ID3DBlob* blob = mVertexShader->GetD3DBlob();
		ID3DBlob* sBlob = mPixelShader->GetD3DBlob();

		mPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ PBRLayout, _countof(PBRLayout) });
		mPSO->SetRootSignature(*mSig);
		mPSO->SetVertexShader(CD3DX12_SHADER_BYTECODE(blob));
		mPSO->SetPixelShader(CD3DX12_SHADER_BYTECODE(sBlob));
		mPSO->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		mPSO->SetRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
		mPSO->SetSampleMask(0xffffffff);

		D3D12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterDesc.FrontCounterClockwise = true;
		mPSO->SetRasterizerState(rasterDesc);
		mPSO->SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		mPSO->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));

		mPSO->Finalize();
	}


	{
		// skybox PSO

		mSkyboxPSO = std::make_shared<PipelineStateObject>(mRenderer->GetRenderDevice());
		std::unique_ptr<Kairos::Shader> skyboxVS = std::make_unique<Kairos::Shader>(mRenderer->GetRenderDevice(),
			ShaderCreateInfo(ShaderType::Vertex, filePath + L"skybox.hlsl", "main_vs"));
		std::unique_ptr<Kairos::Shader> skyboxPS = std::make_unique<Kairos::Shader>(mRenderer->GetRenderDevice(),
			ShaderCreateInfo(ShaderType::Pixel, filePath + L"skybox.hlsl", "main_ps"));

		mSkyboxSignature = std::make_shared<Kairos::RootSignature>(mRenderer->m_Device.get(), 2, 1);
		(*mSkyboxSignature)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX, 0);
		(*mSkyboxSignature)[1].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mSkyboxSignature)[1].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

		mSkyboxSignature->InitStaticSampler(0, sampler, D3D12_SHADER_VISIBILITY_PIXEL);
		mSkyboxSignature->Finalize(L"Skybox Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		//skyboxTexture = CreateRef<Texture>(mRenderer->m_Device.get(), "C:/Users/Chris Ting/Desktop/MeshRenderer/Data/textures/")

		mSkyboxPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ skyboxLayout, _countof(skyboxLayout) });
		mSkyboxPSO->SetRootSignature(*mSkyboxSignature);
		mSkyboxPSO->SetVertexShader(CD3DX12_SHADER_BYTECODE(skyboxVS->GetD3DBlob()));
		mSkyboxPSO->SetPixelShader(CD3DX12_SHADER_BYTECODE(skyboxPS->GetD3DBlob()));
		mSkyboxPSO->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		mSkyboxPSO->SetRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
		mSkyboxPSO->SetSampleMask(0xffffffff);


		D3D12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterDesc.FrontCounterClockwise = true;
		mSkyboxPSO->SetRasterizerState(rasterDesc);
		mSkyboxPSO->SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		D3D12_DEPTH_STENCIL_DESC depthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		mSkyboxPSO->SetDepthStencilState(depthDesc);


		mSkyboxPSO->Finalize();
	}

	{
		mMesh = Mesh::CreateFromFile(mRenderer->m_Device.get(), 
			"C:/Users/Chris Ting/Desktop/MeshRenderer/Data/meshes/cube.obj");
		mVBView = mMesh->GetVertexView();
		mIBView = mMesh->GetIndexView();

		mSkyboxMesh = Mesh::CreateFromFile(mRenderer->m_Device.get(),
			"C:/Users/Chris Ting/Desktop/MeshRenderer/Data/meshes/skybox.obj");
		skyboxVView = mSkyboxMesh->GetVertexView();
		skyboxIView = mSkyboxMesh->GetIndexView();
	}



	{
		// build projection and view matrix
		cameraProjMat = XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);


		// set starting camera state
		cameraPosition = Vector3(0.0f, 0.0f, 5.0f);
		cameraTarget = Vector3(0.0f, 0.0f, 0.0f);
		cameraUp = Vector3(0.0f, 1.0f, 0.0f);

		cameraViewMat = XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp);

		mCamera = EditorCamera(XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f), cameraPosition);

		cube1Position = Vector3(0.f, 0.f, 0.f);
		cube1WorldMat = Matrix::CreateTranslation(cube1Position);
		cube1RotMat = Matrix::Identity;
	}

}
