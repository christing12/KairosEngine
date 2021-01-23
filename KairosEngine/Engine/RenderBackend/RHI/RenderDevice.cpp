#include "krspch.h"
#include "RenderDevice.h"
#include "CommandQueue.h"
#include "CommandContext.h"
#include "SwapChain.h"

#include "TextureManager.h"
#include "ShaderManager.h"
#include "BufferManager.h"
#include "PipelineStateManager.h"

namespace Kairos {

	RenderDevice::RenderDevice(Microsoft::WRL::ComPtr<ID3D12Device5> pdDevice)
		: m_dDevice(pdDevice)
		, m_OffRTVAllocator(this)
		, m_OffDSVAllocator(this)
		, m_OffCBVSRVUAVAllocator(this)
		, m_OffSamplerAllocator(this)
		
	{
		m_GraphicsQueue		= CreateScope<GraphicsQueue>(this);
		m_ComputeQueue		= CreateScope<ComputeQueue>(this);
		m_CopyQueue			= CreateScope<CopyQueue>(this);


		m_TextureManager = CreateScope<TextureManager>(this);
		m_BufferManager = CreateScope<BufferManager>(this);
		m_ShaderManager = CreateScope<ShaderManager>(this, Filesystem::GetWorkingDirectory());
		m_PSOManager = CreateScope<PipelineStateManager>(this, m_ShaderManager.get());

		
		m_OnlineCBV = OnlineCBVSRVUAVHeap(this, { 1000, 1000, 1000 });
		m_OnlineSamplerHeap = OnlineSamplerHeap(this, 1000);
	}

	RenderDevice::~RenderDevice()
	{
	}

	DescriptorRange RenderDevice::AllocateDescriptor(DescriptorType type, Uint32 numDescriptors, bool shaderVisible)
	{
		if (!shaderVisible) {
			switch (type)
			{
			case DescriptorType::RTV: return m_OffRTVAllocator.Allocate(numDescriptors);
			case DescriptorType::DSV: return m_OffDSVAllocator.Allocate(numDescriptors);
			case DescriptorType::Sampler: return m_OffSamplerAllocator.Allocate(numDescriptors);
			case DescriptorType::CBV:
			case DescriptorType::SRV:
			case DescriptorType::UAV:
				return m_OffCBVSRVUAVAllocator.Allocate(numDescriptors);
			}
		}
		else {
			switch (type)
			{
			case DescriptorType::Sampler: return m_OnlineSamplerHeap.AllocateSampler();
			case DescriptorType::CBV: return m_OnlineCBV.AllocateCBV();
			case DescriptorType::SRV: return m_OnlineCBV.AllocateSRV();
			case DescriptorType::UAV: return m_OnlineCBV.AllocateUAV();
			}
		}
	}

	CommandContext& RenderDevice::AllocateCommandContext(CommandType type)
	{
		CommandContext* ctx = m_ContextManager.AllocateContext(this, type, GetCommandQueue(type));

		//TODO: Move Somewhere else
		ctx->SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_OnlineCBV.D3DHeap());
		ctx->SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, m_OnlineSamplerHeap.D3DHeap());
		return *ctx;
	}

	CommandQueue& RenderDevice::GetCommandQueue(CommandType type)
	{
		switch (type)
		{
		case CommandType::Copy: return *m_CopyQueue.get();
		case CommandType::Compute: return *m_ComputeQueue.get();
		case CommandType::Graphics: return *m_GraphicsQueue.get();
		default:
			KRS_CORE_ERROR("Not a valid command type");
			break;
		}
	}

	void RenderDevice::FreeCommandContext(CommandContext& context)
	{
		m_ContextManager.FreeContext(&context);
	}

	void RenderDevice::Flush()
	{
		m_ComputeQueue->Flush();
		m_GraphicsQueue->Flush();
		m_CopyQueue->Flush();
	}

	void RenderDevice::Shutdown()
	{
		m_ContextManager.Shutdown();
		m_CopyQueue->Shutdown();
		m_GraphicsQueue->Shutdown();
		m_ComputeQueue->Shutdown();

		m_TextureManager->Shutdown();
		m_BufferManager->Shutdown();

		UploadResourceAllocator::Destroy();
		m_dDevice = nullptr;
	}

	void RenderDevice::TransientCommand(std::function<void(CommandContext&)> func, CommandType type)
	{
		CommandContext& context = AllocateCommandContext(type);
		func(context);
		context.Submit(true);
	}

	bool RenderDevice::isFenceComplete(Uint64 fenceValue)
	{
		return GetCommandQueue(CommandType(fenceValue >> 56)).isFenceComplete(fenceValue);
	}

	void RenderDevice::Present(SwapChain* swapChain)
	{
	//	Uint32 index = swapChain->CurrBackBuffer();
	//	Kairos::Texture* tex = swapChain->GetBackBuffer(index);


	//	GraphicsContext& ctx = AllocateCommandContext(CommandType::Graphics).GetGraphicsContext();


	////	ctx.TransitionResource(tex, ResourceState::Present, false);

	//	ctx.ClearColor(*tex);
	//	ctx.ClearDepth(*tex, D3D12_CLEAR_FLAG_DEPTH, 1.0, 0);


	//	ctx.Submit(true);


		swapChain->Present();
	}

	RenderHandle RenderDevice::CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource>, const struct TextureProperties& props)
	{
		return RenderHandle{};
	}

	RenderHandle RenderDevice::CreateRootSignature(const std::string& name, std::function<void(RootSignatureProxy&)> fn)
	{
		return m_PSOManager->CreateRootSignature(name, fn).Handle;
	}

	RenderHandle RenderDevice::CreateGraphicsPSO(const std::string& name, std::function<void(struct GraphicsPipelineProxy&)> fn, bool compile)
	{
		PSOQueryResult result = m_PSOManager->CreateGraphicsPipelineState(name, fn);
		if (compile)
			result.Pipeline->Finalize();
		return result.Handle;
	}

	RenderHandle RenderDevice::CreateComputePSO(const std::string& name, std::function<void(struct ComputePiplineProxy&)> fn, bool compile)
	{
		PSOQueryResult result = m_PSOManager->CreateComputePipelineState(name, fn);
		if (compile)
			result.Pipeline->Finalize();
		return result.Handle;
	}

	Descriptor RenderDevice::CreateSampler(const D3D12_SAMPLER_DESC& desc)
	{
		Descriptor result = AllocateDescriptor(DescriptorType::Sampler, 1, true).GetDescriptor(0);

		m_dDevice->CreateSampler(&desc, result.CPUHandle());
		return result;
	}

	void RenderDevice::CompileAll()
	{
		m_PSOManager->CompileAll();
	}


	Buffer* RenderDevice::CreateStaticBuffer(Uint32 size, Uint32 stride, void* data)
	{
		return m_BufferManager->AllocStaticBuffer(size, stride, data).AllocBuffer;
	}

	Texture* RenderDevice::CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, ResourceState state)
	{
		return m_TextureManager->CreateTexture(resource, state).Texture;
	}
	Texture* RenderDevice::CreateTexture(const TextureProperties& props, ResourceState state)
	{
		return m_TextureManager->CreateTexture(props, state).Texture;
	}
	Texture* RenderDevice::CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth, Uint32 mipLevels)
	{
		return m_TextureManager->CreateTexture(filename, format, depth, mipLevels).Texture;
	}
	Buffer* RenderDevice::CreateDynConstBuffer(Uint32 sizeinBytes, Uint32 stride, void* data)
	{
		return m_BufferManager->AllocDynConstBuffer(sizeinBytes, stride, data).AllocBuffer;
	}
}