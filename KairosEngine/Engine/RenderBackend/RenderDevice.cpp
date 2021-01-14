#include "krspch.h"
#include "RenderDevice.h"
#include "Fence.h"
#include "PipelineState.h"
#include "CommandContext.h"
#include "Texture.h"
#include "Buffer.h"

namespace Kairos {
	RenderDevice::RenderDevice(ComPtr<ID3D12Device2> pDevice)
		: m_dDevice(pDevice)
		, m_BufferManager(this)
		, m_TextureManager(this)
		, m_ShaderManager(this, Filesystem::GetWorkingDirectory())
		//, m_PSOManager(this, &m_ShaderManager)
	{
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_DescriptorAllocators[i] = CreateRef<DescriptorAllocator>(this, D3D12_DESCRIPTOR_HEAP_TYPE(i));
		}
		CreateCommandQueues();
	//	m_BufferManager = BufferManager(this);
		m_PSOManager = PipelineStateManager(this, &m_ShaderManager);
	}

	void RenderDevice::Shutdown()
	{
		m_ContextManager.Shutdown();
		m_GraphicsQueue->Shutdown();
		m_CopyQueue->Shutdown();
		m_ComputeQueue->Shutdown();

		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_DescriptorAllocators[i].reset();
		}

		m_dDevice = nullptr;
	}


	void RenderDevice::CreateCommandQueues()
	{
		m_GraphicsQueue		= CreateScope<CommandQueue>(this, D3D12_COMMAND_LIST_TYPE_DIRECT, L"Graphics Queue");
		m_ComputeQueue		= CreateScope<CommandQueue>(this, D3D12_COMMAND_LIST_TYPE_COMPUTE, L"Compute Queue");
		m_CopyQueue			= CreateScope<CommandQueue>(this, D3D12_COMMAND_LIST_TYPE_COPY, L"Copy Queue");
	}

	void RenderDevice::FreeCommandContext(CommandContext* pContext)
	{
		m_ContextManager.FreeCommandContext(pContext);
	}



	DescriptorAllocation RenderDevice::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, Uint32 count)
	{
		return m_DescriptorAllocators[heapType]->Allocate(count);
	}

	void RenderDevice::ReleaseStaleDescriptors()
	{
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_DescriptorAllocators[i]->ReleaseStaleDescriptors();
		}
	}

	void RenderDevice::Flush()
	{
		m_ComputeQueue->Flush();
		m_GraphicsQueue->Flush();
		m_CopyQueue->Flush();
	}

	void RenderDevice::CreateFence(Ref<class Fence>& ppFence)
	{
		ppFence = CreateRef<Fence>(this);
	}

	void RenderDevice::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator)
	{
		switch (type)
		{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:	m_GraphicsQueue->GetFreeAllocator(allocator);	break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:	m_ComputeQueue->GetFreeAllocator(allocator);	break;
			case D3D12_COMMAND_LIST_TYPE_COPY:		m_CopyQueue->GetFreeAllocator(allocator);		break;
		}
		KRS_CORE_ASSERT(allocator != nullptr, "Issue with getting a non nullptr command allocator");
		auto hr = m_dDevice->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(commandList));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating a new D3D Command List");
	}








	/* ---------------------------------- CONTEXT MANAGER STUFF ----------------------------------------*/

	CommandQueue* RenderDevice::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type)
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return m_GraphicsQueue.get();
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return m_CopyQueue.get();
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_ComputeQueue.get();
		default:
			return m_GraphicsQueue.get();
		}
	}

	RenderHandle RenderDevice::CreateTexture(D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal)
	{
		return m_TextureManager.CreateTexture(desc, initState, clearVal);
	}

	RenderHandle RenderDevice::CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState)
	{
		return m_TextureManager.CreateTexture(resource, initState);
	}

	RenderHandle RenderDevice::CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth, Uint32 levels, D3D12_RESOURCE_FLAGS flags)
	{
		return m_TextureManager.CreateTexture(filename, format, depth, levels, flags);
	}


	CommandContext& RenderDevice::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type)
	{
		CommandContext* context = m_ContextManager.AllocateCommandContext(type, this, GetCommandQueue(type));
		return *context;
	}

	CommandContext* ContextManager::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type, RenderDevice* pDevice, CommandQueue* commandQueue)
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);

		auto& contextOfType = m_FreeContexts[type];
		CommandContext* pCommandContext;
		if (!contextOfType.empty()) {
			pCommandContext = contextOfType.front();
			contextOfType.pop();
			pCommandContext->Reset(pDevice->GetCommandQueue(type));
		}
		else {
			pCommandContext = new CommandContext(pDevice, type);
			m_ContexPool[type].emplace_back(pCommandContext);
		}
		return pCommandContext;
	}
	void ContextManager::FreeCommandContext(CommandContext* pContext)
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		m_FreeContexts[pContext->GetType()].push(pContext);
	}
	void ContextManager::Shutdown()
	{
		for (size_t i = 0; i < 4; ++i)
			m_ContexPool[i].clear();
	}


	void RenderDevice::TransientCommand(std::function<void(CommandContext& cmd)>&& function)
	{
		CommandContext ctx = AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		function(ctx);
		ctx.Submit(true);
	}

	void RenderDevice::CompileAll()
	{
		m_PSOManager.CompileAll();
	}

	RenderHandle RenderDevice::CreateVertBuffer(Uint32 numElements, Uint32 stride, void* data)
	{
		
		return m_BufferManager.AllocVertBuffer(numElements, stride, data);
	}

	RenderHandle RenderDevice::CreateIndexBuffer(Uint32 numElements, Uint32 stride, void* data)
	{

		return m_BufferManager.AllocIndexBuffer(numElements, stride, data);

	}

	RenderHandle RenderDevice::CreateGraphicsPSO(const std::string name, std::function<void(GraphicsPipelineProxy&)> fn)
	{
		return m_PSOManager.CreateGraphicsPipelineState(name, fn);
	}

	RenderHandle RenderDevice::CreateComputePSO(const std::string name, std::function<void(ComputePiplineProxy&)> fn)
	{
		return m_PSOManager.CreateComputePipelineState(name, fn);
	}

	RenderHandle RenderDevice::CreateRootSignature(const std::string name, std::function<void(class RootSignature&)> fn)
	{
		return m_PSOManager.CreateRootSignature(name, fn);
	}


	bool RenderDevice::IsFenceComplete(Uint64 fenceValue)
	{
		return GetCommandQueue(D3D12_COMMAND_LIST_TYPE(fenceValue >> 56))->IsFenceComplete(fenceValue);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RenderDevice::GetSRV(RenderHandle handle)
	{
		return m_TextureManager.GetSRV(handle);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RenderDevice::GetDSV(RenderHandle handle)
	{
		return m_TextureManager.GetDSV(handle);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RenderDevice::GetRTV(RenderHandle handle)
	{
		return m_TextureManager.GetRTV(handle);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RenderDevice::GetUAV(RenderHandle handle, Uint32 index)
	{
		return m_TextureManager.GetUAV(handle, index);
	}

	/* -------------------------------- END OF CONTEXT MANAGER -------------------------------------------- */
}