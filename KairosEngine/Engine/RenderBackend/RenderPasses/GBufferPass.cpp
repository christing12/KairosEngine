#include "krspch.h"
#include "GBufferPass.h"
#include "RenderBackend/RHI/PipelineStateManager.h"
#include "RenderBackend/ResourceStorage.h"
#include "RenderBackend/RHI/CommandContext.h"
#include "Scene/Scene.h"
#include "Scene/SceneGPUStorage.h"
#include "RenderBackend/RHI/Buffer.h"
#include "RenderBackend/RHI/RenderDevice.h"

namespace Kairos {
	GBufferPass::GBufferPass()
		: RenderPass("GBufferPass")
	{}

	GBufferPass::~GBufferPass()
	{
	}

	void GBufferPass::WriteData(GBufferPassConstants & constants)
	{
		m_ConstBuffer->Write(constants);
	}

	void GBufferPass::Setup(RenderDevice* pDevice, PipelineStateManager* manager)
	{
		m_Device = pDevice;
		manager->CreateRootSignature("Base", [](RootSignatureProxy& sig)
			{
				//sig.AddRootConstant(std::ceil(sizeof(Uint32) / 4.f), 0, 0);
				sig.AddSRParam(0, 0);
				sig.AddSRParam(1, 0);
				sig.AddSRParam(2, 0);
				sig.AddSRParam(3, 0);
			});

		manager->CreateGraphicsPipelineState("GBufferBase", [&](GraphicsPipelineProxy& proxy)
			{
				proxy.VSFile = "test.hlsl";
				proxy.PSFile = "test.hlsl";
				proxy.RootSigName = "Base";
				proxy.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
				proxy.RenderTargetFormats = {
					DXGI_FORMAT_R16G16B16A16_FLOAT,
				};
			});

		m_ConstBuffer = CreateScope<DynamicBuffer>(m_Device, BufferProperties(sizeof(GBufferPassConstants), sizeof(GBufferPassConstants), 0));
	}

	void GBufferPass::Execute(CommandContext& context, ResourceStorage& storage, CPVoid data, Uint32 sizeInBytes)
	{
		GraphicsContext& gfxCtx = context.GetGraphicsContext();
		auto sceneInfo = gfxCtx.GetScene()->GPUStorage();

		gfxCtx.SetPipelineState("GBufferBase");
		gfxCtx.BindBuffer(*sceneInfo->MergedVertices(), 0, 0, ShaderRegister::ShaderResource);
		gfxCtx.BindBuffer(*sceneInfo->MergedIndices(), 1, 0, ShaderRegister::ShaderResource);
		gfxCtx.BindBuffer(*sceneInfo->MatTableBuffer(), 2, 0, ShaderRegister::ShaderResource);
		gfxCtx.BindBuffer(*sceneInfo->LightTableBuffer(), 3, 0, ShaderRegister::ShaderResource);
		//for (Mesh& mesh : gfxCtx.GetScene()->MergedMeshes)
		gfxCtx.BindPassConstantBuffer(m_ConstBuffer->GPUVirtualAdress());
		// TODO: Add Ability for more meshes than just 1st one
		
		gfxCtx.DrawInstanced(gfxCtx.GetScene()->MergedMeshes()[0].Indices().size(), 1, 0, 0);
	}


}


