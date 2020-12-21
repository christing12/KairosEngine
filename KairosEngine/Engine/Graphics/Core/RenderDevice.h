#pragma once
#include "DescriptorHeap.h"
/*
	Logical device for graphics API
	Command Allocator Pool stored here because this is what usually allocates them
*/
namespace Kairos {
	class CommandQueue;
	class CommandContext;
	class Texture;
	class Buffer;

	class ContextManager {
	public:
		ContextManager() {}

		CommandContext* AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type,
			class RenderDevice* pDevice, Ref<class CommandQueue> commandQueue);
		void FreeCommandContext(CommandContext* pContext);

		void Shutdown();
	private:
		using ContextPool = std::vector<Ref<CommandContext> >;
		using ContextQueue = std::queue<CommandContext*>;

		ContextPool m_ContexPool[4];
		ContextQueue m_FreeContexts[4];

		std::mutex m_AllocationMutex;
	};


	class RenderDevice : public std::enable_shared_from_this<RenderDevice> {
	public:
		friend class ContextManager;

		RenderDevice(ID3D12Device2* pdDevice, IDXGIAdapter4* pdAdapater = nullptr);
		ID3D12Device2* GetD3DDevice() { return m_dDevice.Get(); }
		IDXGIAdapter4* GetDXGIAdapater() { return m_Adapter.Get(); }
		Ref<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	public:
		void Shutdown();
		void CreateCommandQueues();
		void CreatePipelineState(Ref<class PipelineStateObject>& ppPSO);
		void CreateFence(Ref<class Fence>& ppFence);
		void CreateTexture(Ref<class Texture>& ppTexture, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal = nullptr);
		void CreateTexture(Ref<class Texture>& ppTexture, Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState);
		void CreateBuffer(Ref<Buffer>& ppBuffer, Uint32 numElements, Uint32 stride, CPVoid initData = nullptr, const std::wstring& debugName = L"Buffer");

		CommandContext& AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type);
		void FreeCommandContext(CommandContext* pContext);
		void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator);

		DescriptorAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, Uint32 count = 1);
		void ReleaseStaleDescriptors();
		void Flush();
	private:
		Microsoft::WRL::ComPtr<ID3D12Device2> m_dDevice;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_Adapter = nullptr;

		ContextManager m_ContextManager; // for command list stuff;

		Ref<CommandQueue> m_GraphicsQueue;
		Ref<CommandQueue> m_CopyQueue;
		Ref<CommandQueue> m_ComputeQueue;

		Ref<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	};
}