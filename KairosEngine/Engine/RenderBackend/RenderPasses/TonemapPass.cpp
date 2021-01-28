#include "krspch.h"
#include "TonemapPass.h"
#include "RenderBackend/RHI/PipelineStateManager.h"
#include "RenderBackend/ResourceStorage.h"

namespace Kairos {
	TonemapPass::TonemapPass()
		: RenderPass("TonemapPass")
	{}

	void TonemapPass::Setup(RenderDevice* pDevice, PipelineStateManager* manager)
	{
		manager->CreateRootSignature("Tonemap", [](RootSignatureProxy& sig)
			{
				sig.AddRootConstant(std::ceil(sizeof(Uint32) / 4.f), 0, 0);
			});

		manager->CreateGraphicsPipelineState("Tonemap", [&](GraphicsPipelineProxy& proxy)
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
	}

	void TonemapPass::Execute(CommandContext& context, ResourceStorage& storage, CPVoid data, Uint32 size)
	{
		
	}


}


