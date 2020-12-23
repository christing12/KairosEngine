#include "apppch.h"
#include "MeshRenderer.h"

using namespace Kairos;

static D3D12_INPUT_ELEMENT_DESC inputLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	gfxContext.SetPipelineState(*(mPSO.get()));
	gfxContext.SetRootSignature(*(mSig.get()));

	gfxContext.SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfxContext.SetVertexBuffers(0, 1, mVBView);
	gfxContext.SetIndexBuffer(mIBView);

	gfxContext.SetDynamicDescriptors(1, 0, 1, mTexture->GetSRV());
	TestCubes(gfxContext);

	mEditor.Render(gfxContext);

	gfxContext.Submit(true);
}

void MeshRenderer::Present()
{
	mRenderer->Present();
}

void MeshRenderer::TestCubes(Kairos::GraphicsContext& context)
{
	{
		// rotation matrices
		Matrix rotXMat = Matrix::CreateRotationX(0.001f);
		Matrix rotYMat = Matrix::CreateRotationY(0.002f);
		Matrix rotZMat = Matrix::CreateRotationZ(0.0003f);

		Matrix rotMat = cube1RotMat * rotXMat * rotYMat * rotZMat;
		cube1RotMat = rotMat;

		Matrix translationMat = Matrix::CreateTranslation(cube1Position);

		Matrix worldMat = rotMat * translationMat;
		cube1WorldMat = worldMat;

		Matrix wvpMat = cube1WorldMat * mCamera.GetViewMat() * mCamera.GetProjMat();
		cbPerObject.mvpMat = wvpMat.Transpose();

		context.SetDynamicCBV(0, sizeof(cbPerObject), &cbPerObject);
		context.DrawIndexedInstance(mMesh->m_Indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::InitEngine()
{
	float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
	mEditor = EngineIMGUI(mRenderer->m_Device.get(), mWindow.GetNativeWindow());
	mCamera = EditorCamera(XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f));
	

	auto* pDevice = mRenderer->GetDevice();
	{
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		mSig = CreateRef<RootSignature>(mRenderer->m_Device.get(), 2, 1);

		(*mSig)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);

		(*mSig)[1].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_PIXEL);
		(*mSig)[1].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

		D3D12_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.MinLOD = 0.f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;

		mSig->InitStaticSampler(0, sampler, D3D12_SHADER_VISIBILITY_PIXEL);


		mTexture = CreateRef<Texture>(mRenderer->GetRenderDevice(),
			"C:/Users/Chris Ting/Desktop/MeshRenderer/Data/Textures/wall.jpg");
		mSig->Finalize(L"ROOT SIGNATURE", rootSignatureFlags);
	}

	{
		std::wstring filePath = L"C:/Users/Chris Ting/Desktop/MeshRenderer/KairosEngine/Engine/Graphics/Shaders/";

		ShaderCreateInfo Vinfo(ShaderType::Vertex, filePath + L"VertexShader.hlsl", "main");
		mVertexShader = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), Vinfo);

		ShaderCreateInfo PInfo(ShaderType::Pixel, filePath + L"PixelShader.hlsl", "main");
		mPixelShader = CreateRef<Kairos::Shader>(mRenderer->GetRenderDevice(), PInfo);

		mPSO = CreateRef<PipelineStateObject>(mRenderer->GetRenderDevice());

		ID3DBlob* blob = mVertexShader->GetD3DBlob();
		ID3DBlob* sBlob = mPixelShader->GetD3DBlob();

		mPSO->SetInputLayout(D3D12_INPUT_LAYOUT_DESC{ inputLayout, _countof(inputLayout) });
		mPSO->SetRootSignature(mSig->GetD3DRootSignature());
		mPSO->SetVertexShader(CD3DX12_SHADER_BYTECODE(blob));
		mPSO->SetPixelShader(CD3DX12_SHADER_BYTECODE(sBlob));
		mPSO->SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		mPSO->SetRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
		mPSO->SetSampleMask(0xffffffff);
		mPSO->SetRasterizerState(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
		mPSO->SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		mPSO->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));

		mPSO->Finalize();
	}


	{
		mMesh = CreateRef<Mesh>(mRenderer->m_Device.get(), "C:/Users/Chris Ting/Desktop/MeshRenderer/Data/Chair.obj");
		mVBView = mMesh->GetVertexView();
		mIBView = mMesh->GetIndexView();
	}

	{
		// build projection and view matrix
		cameraProjMat = XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);

		// set starting camera state
		cameraPosition = Vector3(0.0f, 0.0f, 4.0f);
		std::cout << cameraPosition.Length() << std::endl;
		cameraTarget = Vector3(0.0f, 0.0f, 0.0f);
		cameraUp = Vector3(0.0f, 1.0f, 0.0f);

		cameraViewMat = XMMatrixLookAtLH(cameraPosition, cameraTarget, cameraUp);
		Matrix test = mCamera.GetViewMat();

		cube1Position = Vector3(0.f, 0.f, 0.f);
		cube1WorldMat = Matrix::CreateTranslation(cube1Position);
		cube1RotMat = Matrix::Identity;
	}

}
