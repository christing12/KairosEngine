#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>


KRS_BEGIN_NAMESPACE(Kairos)

class RootSignature;
class PipelineState;
class ShaderManager;

// temporary object to test pipeline configuration
struct GraphicsPipelineProxy {
	std::string VSFile;
	std::string PSFile;
	std::optional<std::string> GSFile;
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
	std::optional<std::string> RootSignatureName;
};




/* Pipeline State Manager */

class PipelineStateManager {
public:
	friend class CommandContext;
	KRS_CLASS_NON_COPYABLE_AND_MOVABLE(PipelineStateManager);


	PipelineStateManager(
		class RenderDevice* pDevice,
		Ref<class ShaderManager> shaderManager
	);

	using PSOName = std::string;
	using RSName = std::string;
	using GraphicsPipelineFn = std::function<void(GraphicsPipelineProxy&)>;
	using ComputePipelineFn = std::function<void(ComputePiplineProxy&)>;
	using PSOHandle = std::optional<RenderHandle>;
	using RSHandle = std::optional<RenderHandle>;
	using PipelineNameCache = std::unordered_map<std::string, RenderHandle>;
	using PipelineCache = std::vector<PipelineState>;

	void CreateGraphicsPipelineState(PSOName name, const GraphicsPipelineFn& pipelineFn);
	void CreateComputePipelineState(PSOName name, const ComputePipelineFn& computeFn);
	//void CreateRootSignature(PSOName name, )

	// checks for name to see if its already been registered
	RSHandle GetRootSignature(RSName name);
	// checks for name to see if its already been registered
	PSOHandle GetPipelineState(PSOName name);


private:
	RenderDevice* m_Device;
	Ref<class ShaderManager> m_ShaderManager;



	PipelineNameCache m_PipelineNameCache;

	PipelineCache m_RegisteredPSO;
	std::vector<PipelineState> m_RegisteredRS;

	std::string defaultVSEntryPoint = "main_vs";
	std::string defaultPSEntryPoint = "main_ps";
	std::string defaultCSentryPoint = "main";
};



KRS_END_NAMESPACE