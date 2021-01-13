#include "krspch.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "PipelineStateManager.h"
#include "Shader.h"
#include "RenderDevice.h"

namespace Kairos {
	/* ---------------------------- PIPELINE MANAGER FUNCTIONS ------------------------*/

	PipelineStateManager::PipelineStateManager(RenderDevice* pDevice, Ref<ShaderManager> shaderManager)
		: m_Device(pDevice)
		, m_ShaderManager(shaderManager)
	{}

	void PipelineStateManager::CreateGraphicsPipelineState(PSOName name, const GraphicsPipelineFn& pipelineFn)
	{
		KRS_CORE_ASSERT(!GetPipelineState(name), "Pipeline State already exists!");

		GraphicsPipelineProxy proxy{};

		pipelineFn(proxy);
		KRS_CORE_ASSERT(!proxy.VSFile.empty(), "No Vertex Shader listed in proxy object");
		KRS_CORE_ASSERT(!proxy.PSFile.empty(), "No Pixel Shader listed in proxy object");

		GraphicsPSO PSO(m_Device);
		PSO.SetBlendState(proxy.BlendState);
		PSO.SetDepthStencilState(proxy.DepthStencilState);
		PSO.SetSampleMask(0xFFFFFFFF);
		PSO.SetInputLayout(proxy.InputLayout);
		PSO.SetTopology(proxy.Topology);
		PSO.SetRenderTargets(proxy.RenderTargetFormats.size(), &proxy.RenderTargetFormats[0], proxy.DepthStencilFormat);
		PSO.SetVertexShader(m_ShaderManager->LoadShader(ShaderType::Vertex, defaultVSEntryPoint, proxy.VSFile));
		PSO.SetPixelShader(m_ShaderManager->LoadShader(ShaderType::Pixel, defaultPSEntryPoint, proxy.PSFile));
		PSO.SetRootSignature(/* Root Signature*/);

		PSO.SetDebugName(name);
		m_RegisteredPSO.emplace_back(std::move(PSO)); // move operator instead of copy

	}

	std::optional<RenderHandle> PipelineStateManager::GetRootSignature(RSName name)
	{
		return {};
		//auto iterator = m_RegisteredRS.find(name);

	}

	PipelineStateManager::PSOHandle PipelineStateManager::GetPipelineState(PSOName name)
	{
		auto it = m_PipelineNameCache.find(name);
		if (it != m_PipelineNameCache.end()) {
			return it->second;
		}

		return std::nullopt;
	}

}