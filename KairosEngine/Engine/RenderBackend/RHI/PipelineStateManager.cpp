#include "krspch.h"
#include "PipelineStateManager.h"
#include "RenderDevice.h"
#include "ShaderManager.h"

namespace Kairos {
	/* ---------------------------- PIPELINE MANAGER FUNCTIONS ------------------------*/

	PipelineStateManager::PipelineStateManager(RenderDevice* pDevice, ShaderManager* shaderManager)
		: m_Device(pDevice)
		, m_ShaderManager(shaderManager)
		, m_DefaultRootSig(pDevice)
	{
		AddGlobalRootSigParam(m_DefaultRootSig);
	}

	RootSigQueryResult PipelineStateManager::CreateRootSignature(const std::string& name, const RootSignatureFn& sigFn)
	{

		RootSignatureProxy proxy;
		sigFn(proxy);

		RootSignature sig(m_Device);

		for (const auto& param : proxy.RootConstantsParameters())
		{
			sig.AddConstants(param);
		}

		for (const auto& param : proxy.RootConstantBufferParameters())
		{
			sig.AddDescriptor(param);
		}

		for (const auto& param : proxy.RootDescriptorTableParameters())
		{
			sig.AddDescriptorTable(param);
		}

		AddGlobalRootSigParam(sig);


		sig.Finalize(D3D12_ROOT_SIGNATURE_FLAG_NONE | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::RootSignature) | (m_RegisteredRS.size() << 8) };

		m_RegisteredRS.emplace_back(CreateScope<RootSignature>(sig));
		m_RootSigNameCache.insert(std::pair<std::string, RenderHandle>(name, handle));
		
		return RootSigQueryResult{ handle, m_RegisteredRS.back().get() };
	}

	PSOQueryResult PipelineStateManager::CreateGraphicsPipelineState(PSOName name, const GraphicsPipelineFn& pipelineFn)
	{
		//	KRS_CORE_ASSERT(GetPipelineState(name).Valid(), "Pipeline State already exists!");

		GraphicsPipelineProxy proxy{};

		proxy.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		proxy.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		proxy.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		D3D12_RASTERIZER_DESC desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		desc.FrontCounterClockwise = true;
		proxy.RasterizerState = desc;
		proxy.Topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		pipelineFn(proxy);

		KRS_CORE_ASSERT(!proxy.VSFile.empty(), "No Vertex Shader listed in proxy object");
		KRS_CORE_ASSERT(!proxy.PSFile.empty(), "No Pixel Shader listed in proxy object");

		GraphicsPSO PSO(m_Device);
		PSO.SetBlendState(proxy.BlendState);
		PSO.SetDepthStencilState(proxy.DepthStencilState);
		PSO.SetSampleMask(0xFFFFFFFF);
		PSO.SetTopology(proxy.Topology);
		PSO.SetRasterizerState(desc);
		PSO.SetRenderTargets(proxy.RenderTargetFormats.size(), &proxy.RenderTargetFormats[0], proxy.DepthStencilFormat);


		Shader* vs = m_ShaderManager->LoadShader(ShaderType::Vertex, defaultVSEntryPoint, proxy.VSFile);
		Shader* ps = m_ShaderManager->LoadShader(ShaderType::Pixel, defaultPSEntryPoint, proxy.PSFile);
		PSO.SetVertexShader(vs);
		PSO.SetPixelShader(ps);
		PSO.SetRootSignature(GetRootSignature(proxy.RootSigName).Signature);

		PSO.SetDebugName(name);
		m_CachedGraphicsPSO.emplace_back(CreateScope<GraphicsPSO>(std::move(PSO)));

		RenderHandle handle = FormPSOHandle(true, m_CachedGraphicsPSO.size() - 1);
		m_PipelineNameCache.insert(std::pair<std::string, RenderHandle>(name, handle));
		return PSOQueryResult{ handle, m_CachedGraphicsPSO[m_CachedGraphicsPSO.size() - 1].get() };
	}

	PSOQueryResult PipelineStateManager::CreateComputePipelineState(PSOName name, const ComputePipelineFn& computeFn)
	{
		ComputePiplineProxy proxy{};

		computeFn(proxy);

		ComputePSO PSO(m_Device);
		std::string entryPoint = proxy.EntryPoint.has_value() ? proxy.EntryPoint.value() : defaultCSentryPoint;

		Shader* cs = m_ShaderManager->LoadShader(ShaderType::Compute, entryPoint, proxy.CSFile);
		PSO.SetComputeShader(cs);
		PSO.SetDebugName(name);
		PSO.SetRootSignature(GetRootSignature(proxy.RootSignatureName).Signature);
		m_CachedComputePSO.emplace_back(CreateScope<ComputePSO>(std::move(PSO)));

		RenderHandle handle = FormPSOHandle(false, m_CachedComputePSO.size() - 1);
		m_PipelineNameCache.insert(std::pair<std::string, RenderHandle>(name, handle));
		return PSOQueryResult{ handle, m_CachedComputePSO.back().get() };
	}

	void PipelineStateManager::CompileAll()
	{
		//for (RootSignature& sig : m_RegisteredRS)
		//{
		//	sig.Finalize(D3D12_ROOT_SIGNATURE_FLAG_NONE);
		//}

		for (auto& state : m_CachedGraphicsPSO)
		{
			state->Finalize();
		}

		for (auto& state : m_CachedComputePSO)
		{
			state->Finalize();
		}
	}

	PSOQueryResult PipelineStateManager::GetPipelineState(PSOName name)
	{
		PSOQueryResult result = {};
		RenderHandle handle{};
		auto it = m_PipelineNameCache.find(name);
		if (it != m_PipelineNameCache.end()) {
			handle = it->second;
		}
		Uint64 idx = PSOIndexFromHandle(handle);

		bool graphics = ((1 << 8) & handle.handle) != 0;
		if (graphics)
			return PSOQueryResult{ handle, m_CachedGraphicsPSO[idx].get() };
		else
			return PSOQueryResult{ handle, m_CachedComputePSO[idx].get() };
	}

	RenderHandle PipelineStateManager::FormPSOHandle(bool graphics, Uint64 index)
	{
		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::PSO) };
		handle.handle |= graphics ? (1 << 8) : (0 << 8);
		handle.handle |= (((graphics ? m_CachedGraphicsPSO.size() : m_CachedComputePSO.size()) - 1) << 9);
		return handle;
	}

	void PipelineStateManager::AddGlobalRootSigParam(RootSignature& signature)
	{
		// Global data CB
		signature.AddDescriptor(RootDescriptor{ 0, 10, D3D12_ROOT_PARAMETER_TYPE_CBV, ShaderRegister::ConstantBuffer });

		// Frame-specific data CB
		signature.AddDescriptor(RootDescriptor{ 1, 10, D3D12_ROOT_PARAMETER_TYPE_CBV, ShaderRegister::ConstantBuffer });

		// Pass-specific data CB
		signature.AddDescriptor(RootDescriptor{ 2, 10, D3D12_ROOT_PARAMETER_TYPE_CBV, ShaderRegister::ConstantBuffer });

		// Unbounded Texture2D range
		RootDescriptorTable textures2D;
		textures2D.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::ShaderResource, 0, 10 });
		signature.AddDescriptorTable(textures2D);

		// Unbounded Texture2D<uint4> range
		RootDescriptorTable textures2DUInt4;
		textures2DUInt4.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::ShaderResource, 0, 11 });
		signature.AddDescriptorTable(textures2DUInt4);

		// Unbounded Texture3D range
		RootDescriptorTable textures3D;
		textures3D.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::ShaderResource, 0, 12 });
		signature.AddDescriptorTable(textures3D);

		// Unbounded Texture3D<uint4> range
		RootDescriptorTable textures3DUInt4;
		textures3DUInt4.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::ShaderResource, 0, 13 });
		signature.AddDescriptorTable(textures3DUInt4);

		// Unbounded Texture2DArray range
		RootDescriptorTable texture2DArrays;
		texture2DArrays.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::ShaderResource, 0, 14 });
		signature.AddDescriptorTable(texture2DArrays);

		// TextureCube range
		RootDescriptorTable textureCube;
		textureCube.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::ShaderResource, 0, 15 });
		signature.AddDescriptorTable(textureCube);




		// Unbounded RWTexture2D range
		RootDescriptorTable RWTextures2DFloat4;
		RWTextures2DFloat4.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 10 });
		signature.AddDescriptorTable(RWTextures2DFloat4);

		// Unbounded RWTexture2D<uint4> range
		RootDescriptorTable RWTextures2DUInt4;
		RWTextures2DUInt4.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 11 });
		signature.AddDescriptorTable(RWTextures2DUInt4);

		// Unbounded RWTexture2D<uint> range
		RootDescriptorTable RWTextures2DUInt;
		RWTextures2DUInt.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 12 });
		signature.AddDescriptorTable(RWTextures2DUInt);

		// Unbounded RWTexture3D range
		RootDescriptorTable RWTextures3D;
		RWTextures3D.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 13 });
		signature.AddDescriptorTable(RWTextures3D);

		// Unbounded RWTexture3D<uint4> range
		RootDescriptorTable RWTextures3DUInt4;
		RWTextures3DUInt4.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 14 });
		signature.AddDescriptorTable(RWTextures3DUInt4);

		// Unbounded RWTexture2DArray range
		RootDescriptorTable RWTexture2DArrays;
		RWTexture2DArrays.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 15 });
		signature.AddDescriptorTable(RWTexture2DArrays);


		RootDescriptorTable RWTexture2DFloat2;
		RWTexture2DFloat2.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::UnorderedAcces, 0, 16 });
		signature.AddDescriptorTable(RWTexture2DFloat2);




		//Unbounded Samplers range
		RootDescriptorTable samplers;
		samplers.AddDescriptorRange(RootDescriptorTableRange{ ShaderRegister::Sampler, 0, 10 });
		signature.AddDescriptorTable(samplers);

		// Debug readback buffer
		signature.AddDescriptor(RootDescriptor{ 0, 17, D3D12_ROOT_PARAMETER_TYPE_UAV, ShaderRegister::UnorderedAcces });
	}

	RootSigQueryResult PipelineStateManager::GetRootSignature(const std::string& name)
	{
		auto it = m_RootSigNameCache.find(name);
		if (it == m_RootSigNameCache.end()) return RootSigQueryResult{};

		RenderHandle& handle = it->second;

		Uint64 index = RSIndexFromHandle(handle);
		return RootSigQueryResult{ handle, m_RegisteredRS[index].get() };
	}

	void RootSignatureProxy::AddRootConstant(Uint16 numConstants, Uint16 registerIdx, Uint16 space)
	{
		m_Constants.emplace_back(RootConstants(registerIdx, numConstants, space));
	}

	void RootSignatureProxy::AddSRParam(Uint16 registerIdx, Uint16 space)
	{
		RootDescriptorTable& table = m_DescriptorTables.emplace_back();
		table.AddDescriptorRange(RootDescriptorTableRange(ShaderRegister::ShaderResource,
			registerIdx, space, 1));
	}

	void RootSignatureProxy::AddUAParam(Uint16 registerIdx, Uint16 space)
	{
		RootDescriptorTable& table = m_DescriptorTables.emplace_back();
		table.AddDescriptorRange(RootDescriptorTableRange(ShaderRegister::UnorderedAcces,
			registerIdx, space, 1));
	}

	void RootSignatureProxy::AddConstantBuffer(Uint16 registerIdx, Uint16 space)
	{
		m_ConstantBuffers.emplace_back(RootDescriptor(registerIdx, space, D3D12_ROOT_PARAMETER_TYPE_CBV, ShaderRegister::ConstantBuffer));
	}

}