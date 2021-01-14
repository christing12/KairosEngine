#include "krspch.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "PipelineStateManager.h"
#include "Shader.h"
#include "RenderDevice.h"

namespace Kairos {
	/* ---------------------------- PIPELINE MANAGER FUNCTIONS ------------------------*/

	PipelineStateManager::PipelineStateManager(RenderDevice* pDevice, ShaderManager* shaderManager)
		: m_Device(pDevice)
		, m_ShaderManager(shaderManager)
	{}

	RenderHandle PipelineStateManager::CreateRootSignature(const std::string& name, const RootSignatureFn& sigFn)
	{
		RootSignature sig;
		
		sigFn(sig);

		AddGlobalRootSigParam(sig);

		m_RegisteredRS.emplace_back(std::move(sig));
		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::RootSignature) | (1 << (m_RegisteredPSO.size() - 1)) };
		m_RootSigNameCache.insert(std::pair<std::string, RenderHandle>(name, handle));

		return handle;
	}

	RenderHandle PipelineStateManager::CreateGraphicsPipelineState(PSOName name, const GraphicsPipelineFn& pipelineFn)
	{
		KRS_CORE_ASSERT(GetPipelineState(name).Valid(), "Pipeline State already exists!");

		GraphicsPipelineProxy proxy{};

		proxy.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		proxy.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		proxy.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		proxy.Topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;


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
		PSO.SetRootSignature(GetRootSignature(proxy.RootSigName));

		PSO.SetDebugName(name);
		m_RegisteredPSO.emplace_back(std::move(PSO)); // move operator instead of copy


		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::PSO) | (1 << (m_RegisteredPSO.size() - 1)) };
		m_PipelineNameCache.insert(std::pair<std::string, RenderHandle>(name, handle));
		return handle;
	}

	RenderHandle PipelineStateManager::CreateComputePipelineState(PSOName name, const ComputePipelineFn& computeFn)
	{
		return RenderHandle();
	}

	RenderHandle PipelineStateManager::GetPipelineState(PSOName name)
	{
		auto it = m_PipelineNameCache.find(name);
		if (it != m_PipelineNameCache.end()) {
			return it->second;
		}
		return RenderHandle{};
	}

	Uint64 PipelineStateManager::RSIndexFromHandle(const RenderHandle& handle) const
	{
		constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 8) - 1);
		return Uint64((handle.handle & resourceMask) >> 8);
	}

	void PipelineStateManager::AddGlobalRootSigParam(RootSignature& signature)
	{
		// Global data CB
		signature.AddDescriptor(RootDescriptor{ 0, 10, RootDescriptorType::CBV });

		// Frame-specific data CB
		signature.AddDescriptor(RootDescriptor{ 1, 10, RootDescriptorType::CBV });

		// Pass-specific data CB
		signature.AddDescriptor(RootDescriptor{ 2, 10, RootDescriptorType::CBV });

		// Unbounded Texture2D range
		RootDescriptorTable textures2D;
		textures2D.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10 });
		signature.AddDescriptorTable(textures2D);

		// Unbounded Texture2D<uint4> range
		RootDescriptorTable textures2DUInt4;
		textures2DUInt4.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 11 });
		signature.AddDescriptorTable(textures2DUInt4);

		// Unbounded Texture3D range
		RootDescriptorTable textures3D;
		textures3D.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 12 });
		signature.AddDescriptorTable(textures3D);

		// Unbounded Texture3D<uint4> range
		RootDescriptorTable textures3DUInt4;
		textures3DUInt4.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 13 });
		signature.AddDescriptorTable(textures3DUInt4);

		// Unbounded Texture2DArray range
		RootDescriptorTable texture2DArrays;
		texture2DArrays.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 14 });
		signature.AddDescriptorTable(texture2DArrays);

		// Unbounded RWTexture2D range
		RootDescriptorTable RWTextures2DFloat4;
		RWTextures2DFloat4.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 10 });
		signature.AddDescriptorTable(RWTextures2DFloat4);

		// Unbounded RWTexture2D<uint4> range
		RootDescriptorTable RWTextures2DUInt4;
		RWTextures2DUInt4.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 11 });
		signature.AddDescriptorTable(RWTextures2DUInt4);

		// Unbounded RWTexture2D<uint> range
		RootDescriptorTable RWTextures2DUInt;
		RWTextures2DUInt.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 12 });
		signature.AddDescriptorTable(RWTextures2DUInt);

		// Unbounded RWTexture3D range
		RootDescriptorTable RWTextures3D;
		RWTextures3D.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 13 });
		signature.AddDescriptorTable(RWTextures3D);

		// Unbounded RWTexture3D<uint4> range
		RootDescriptorTable RWTextures3DUInt4;
		RWTextures3DUInt4.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 14 });
		signature.AddDescriptorTable(RWTextures3DUInt4);

		// Unbounded RWTexture2DArray range
		RootDescriptorTable RWTexture2DArrays;
		RWTexture2DArrays.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 15 });
		signature.AddDescriptorTable(RWTexture2DArrays);

		// Unbounded Samplers range
		RootDescriptorTable samplers;
		samplers.AddDescriptorRange(RootDescriptorTableRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 10 });
		signature.AddDescriptorTable(samplers);

		// Debug readback buffer
		signature.AddDescriptor(RootDescriptor{ 0, 16, RootDescriptorType::UAV });
	}

	const RootSignature* PipelineStateManager::GetRootSignature(const std::string& name)
	{
		auto it = m_RootSigNameCache.find(name);
		if (it == m_RootSigNameCache.end()) return nullptr;

		RenderHandle& handle = it->second;

		Uint64 index = RSIndexFromHandle(handle);
		return &m_RegisteredRS[index];
	}

}