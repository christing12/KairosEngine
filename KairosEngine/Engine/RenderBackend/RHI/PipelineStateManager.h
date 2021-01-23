#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>
#include "PipelineState.h"
#include "RootSignature.h"

namespace Kairos {

	class RootSignature;
	class PipelineState;
	class GraphicsPSO;
	class ComputePSO;
	class ShaderManager;
	class RootSignature;

	// temporary object to test pipeline configuration
	struct GraphicsPipelineProxy {
		std::string VSFile;
		std::string PSFile;
		std::string RootSigName;
		std::string GSFile;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_DEPTH_STENCIL_DESC DepthStencilState;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology;
		std::vector<DXGI_FORMAT> RenderTargetFormats;
		DXGI_FORMAT DepthStencilFormat;
		D3D12_BLEND_DESC BlendState;
		D3D12_INPUT_LAYOUT_DESC InputLayout;
	};

	struct ComputePiplineProxy {
		std::string CSFile;
		std::string RootSignatureName;
	};

	// for tricky lambda stuff
	class RootSignatureProxy {
	public:
		void AddRootConstant(Uint16 numConstants, Uint16 registerIdx, Uint16 space);
		void AddSRParam(Uint16 registerIdx, Uint16 space);
		void AddUAParam(Uint16 registerIdx, Uint16 space);
		void AddConstantBuffer(Uint16 registerIdx, Uint16 space);
	public:
		std::vector<RootConstants> m_Constants;
		std::vector<RootDescriptor> m_ConstantBuffers;
		std::vector<RootDescriptorTable> m_DescriptorTables;
	public:
		inline const auto& RootConstantsParameters() const { return m_Constants; }
		inline const auto& RootConstantBufferParameters() const { return m_ConstantBuffers; }
		inline const auto& RootDescriptorTableParameters() const { return m_DescriptorTables; }
	};


	struct PSOQueryResult {
		RenderHandle Handle;
		PipelineState* Pipeline;
	};

	struct RootSigQueryResult {
		RenderHandle Handle;
		RootSignature* Signature;
	};

	/* Pipeline State Manager */

	class PipelineStateManager {
	public:
		
		friend class CommandContext;

		using PSOName = std::string;
		using RSName = std::string;
		using GraphicsPipelineFn = std::function<void(GraphicsPipelineProxy&)>;
		using ComputePipelineFn = std::function<void(ComputePiplineProxy&)>;
		using RootSignatureFn = std::function<void(struct RootSignatureProxy&)>;
		using PipelineNameCache = std::unordered_map<std::string, RenderHandle>;
		using PipelineCache = std::vector<PipelineState>;

	public:
		PipelineStateManager() = default;
		PipelineStateManager(RenderDevice* pDevice, ShaderManager* shaderManager);

		RootSigQueryResult CreateRootSignature(const std::string& name, const RootSignatureFn& sigFn);
		PSOQueryResult CreateGraphicsPipelineState(PSOName name, const GraphicsPipelineFn& pipelineFn);
		PSOQueryResult CreateComputePipelineState(PSOName name, const ComputePipelineFn& computeFn);

		// will add functionality for recompile later?
		void CompileAll();
		// checks for name to see if its already been registered
		PSOQueryResult GetPipelineState(PSOName name);

	private:
		RenderDevice* m_Device;
		ShaderManager* m_ShaderManager;

		std::unordered_map<std::string, RenderHandle> m_RootSigNameCache;
		PipelineNameCache m_PipelineNameCache;

		std::vector<Scope<GraphicsPSO>> m_CachedGraphicsPSO;
		std::vector<Scope<ComputePSO>> m_CachedComputePSO;
		std::vector<Scope<RootSignature>> m_RegisteredRS;

		// default
		RootSignature m_DefaultRootSig;
		std::string defaultVSEntryPoint = "main_vs";
		std::string defaultPSEntryPoint = "main_ps";
		std::string defaultCSentryPoint = "main";

	private:
		RenderHandle FormPSOHandle(bool graphics, Uint64 index);
		void AddGlobalRootSigParam(class RootSignature& signature);
		RootSigQueryResult GetRootSignature(const std::string& name);

	public:
		inline Uint32 NumCommonRootParamters() const { return m_DefaultRootSig.NumParameters(); }
	};


}