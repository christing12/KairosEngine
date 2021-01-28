#include "krspch.h"
#include "SkyboxPass.h"
#include "RenderBackend/RHI/PipelineStateManager.h"
#include "RenderBackend/ResourceStorage.h"
#include "RenderBackend/RHI/RenderDevice.h"
#include "RenderBackend/RHI/CommandContext.h"

namespace Kairos {
	SkyboxPass::SkyboxPass()
		: RenderPass("TonemapPass")
	{}

	void SkyboxPass::WriteData(SkyboxConstants & constants)
	{
		m_ConstBuffer->Write(constants);
	}

	void SkyboxPass::Setup(RenderDevice* pDevice, PipelineStateManager* manager)
	{
		m_skyboxMesh = Mesh::LoadFromAsset(pDevice, "Data/assets_export/skybox.mesh");
		vertexBuffer = pDevice->CreateStaticBuffer(m_skyboxMesh.Vertices().size() * sizeof(Vertex), sizeof(Vertex), m_skyboxMesh.Vertices().data());
		indexBuffer = pDevice->CreateStaticBuffer(m_skyboxMesh.Indices().size() * sizeof(Uint32), sizeof(Uint32), m_skyboxMesh.Indices().data());

		manager->CreateRootSignature("Skybox", [](RootSignatureProxy& sig)
			{
				sig.AddSRParam(0, 0);
				sig.AddSRParam(1, 0);
			});


		manager->CreateGraphicsPipelineState("Skybox", [](GraphicsPipelineProxy& proxy)
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

		m_ConstBuffer = CreateScope<DynamicBuffer>(pDevice, BufferProperties(sizeof(SkyboxConstants), sizeof(SkyboxConstants), 0));
	}

	void SkyboxPass::Execute(CommandContext& context, ResourceStorage& storage, CPVoid data, Uint32 size)
	{
		GraphicsContext& gfx = context.GetGraphicsContext();
		gfx.SetPipelineState("Skybox");
		gfx.BindBuffer(*vertexBuffer, 0, 0, ShaderRegister::ShaderResource);
		gfx.BindBuffer(*indexBuffer, 1, 0, ShaderRegister::ShaderResource);
		
		gfx.BindPassConstantBuffer(m_ConstBuffer->GPUVirtualAdress());
		gfx.DrawInstanced(m_skyboxMesh.Indices().size(), 1, 0, 0);
		
	}


}


