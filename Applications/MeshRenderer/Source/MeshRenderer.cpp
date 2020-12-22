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

void MeshRenderer::Render()
{
	GraphicsContext& gfxContext = mRenderer->GetGraphicsContext();
	auto& currBackBuffer = mRenderer->GetCurrBackBuffer();

	D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] =
	{
		currBackBuffer.GetRTV()
	};

	gfxContext.SetRenderTargets(_countof(RTVs), RTVs, mRenderer->GetDSV());
	gfxContext.ClearColor(currBackBuffer);
	gfxContext.ClearDepth(mRenderer->GetDepthBuffer(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

	gfxContext.Submit(true);
}

void MeshRenderer::Present()
{
	mRenderer->Present();
}

void MeshRenderer::InitEngine()
{
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


		//mTexture = CreateRef<Texture>(mRenderer->GetRenderDevice(),
		//	"C:/Users/Chris Ting/Desktop/Personal/WaterSimulation/Engine/Source/Graphics/Textures/wall.jpg");
		mSig->Finalize(L"ROOT SIGNATURE", rootSignatureFlags);
	}
}
