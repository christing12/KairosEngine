#include "krspch.h"
#include "RenderDevice.h"
#include "Fence.h"
#include "PipelineStateObject.h"
#include "CommandContext.h"
#include "CommandQueue.h"
#include "Texture.h"
#include "Buffer.h"

namespace Kairos {
	RenderDevice::RenderDevice(ID3D12Device2* pdDevice, IDXGIAdapter4* pdAdapater)
		: m_dDevice(pdDevice)
		, m_Adapter(pdAdapater)
	{
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_DescriptorAllocators[i] = CreateRef<DescriptorAllocator>(this, D3D12_DESCRIPTOR_HEAP_TYPE(i));
		}
	}

	Ref<CommandQueue> RenderDevice::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type)
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return m_GraphicsQueue;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return m_CopyQueue;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_ComputeQueue;
		default:
			return m_GraphicsQueue;
		}
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
		m_GraphicsQueue = CreateRef<CommandQueue>(shared_from_this(), D3D12_COMMAND_LIST_TYPE_DIRECT, L"Graphics Queue");
		m_ComputeQueue = CreateRef<CommandQueue>(shared_from_this(), D3D12_COMMAND_LIST_TYPE_COMPUTE, L"Compute Queue");
		m_CopyQueue = CreateRef<CommandQueue>(shared_from_this(), D3D12_COMMAND_LIST_TYPE_COPY, L"Copy Queue");
	}

	void RenderDevice::CreatePipelineState(Ref<class PipelineStateObject>& ppPSO)
	{
		ppPSO = CreateRef<PipelineStateObject>(this);
	}

	void RenderDevice::CreateFence(Ref<class Fence>& ppFence)
	{
		ppFence = CreateRef<Fence>(this);
	}
	void RenderDevice::CreateTexture(Ref<class Texture>& ppTexture, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal)
	{
		ppTexture = CreateRef<Texture>(this, desc, initState, clearVal);
	}
	void RenderDevice::CreateTexture(Ref<class Texture>& ppTexture, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState)
	{
		ppTexture = CreateRef<Texture>(this, resource, initState);
	}

	void RenderDevice::CreateBuffer(Ref<Buffer>& ppBuffer, Uint32 numElements, Uint32 stride, CPVoid initData, const std::wstring& debugName)
	{
		ppBuffer = CreateRef<Buffer>(this, numElements, stride, initData, debugName);
	}

	CommandContext& RenderDevice::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type)
	{
		CommandContext* context = m_ContextManager.AllocateCommandContext(type, this, GetCommandQueue(type));
		return *context;
	}
	void RenderDevice::FreeCommandContext(CommandContext* pContext)
	{
		m_ContextManager.FreeCommandContext(pContext);
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

	CommandContext* ContextManager::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type, RenderDevice* pDevice, Ref<CommandQueue> commandQueue)
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);

		auto& contextOfType = m_FreeContexts[type];
		CommandContext* pCommandContext;
		if (!contextOfType.empty()) {
			pCommandContext = contextOfType.front();
			if (pCommandContext->GetAllocator() != nullptr) {
				std::cout << " BITCH WATH THE FUCK " << std::endl;
			}
			contextOfType.pop();
			pCommandContext->Reset(commandQueue);
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
}