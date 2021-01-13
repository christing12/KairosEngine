#pragma once
#include "DescriptorHeap.h"
#include "GraphicsTypes.h"
#include "BufferManager.h"
#include "Buffer.h"
#include "CommandQueue.h"
/*
	Logical device for graphics API
	Command Allocator Pool stored here because this is what usually allocates them
*/
KRS_BEGIN_NAMESPACE(Kairos)
class CommandQueue;
class CommandContext;
class Texture;
class Buffer;

class ContextManager {
public:
	KRS_CLASS_DEFAULT(ContextManager);

	CommandContext* AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type,
		class RenderDevice* pDevice, CommandQueue* commandQueue);
	void FreeCommandContext(CommandContext* pContext);

	void Shutdown();
private:
	using ContextPool = std::vector<Ref<CommandContext> >;
	using ContextQueue = std::queue<CommandContext*>;

	ContextPool m_ContexPool[4];
	ContextQueue m_FreeContexts[4];

	std::mutex m_AllocationMutex;
};

class TextureManager {
public:
	KRS_CLASS_NON_COPYABLE(TextureManager);
		
		

private:
		
};


class RenderDevice  {

public:
	friend class ContextManager;
	friend class RenderGraph;
	friend class PipelineStateManager;
	friend class ShaderManager;
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;
	friend class RenderScene;
		

	RenderDevice(Microsoft::WRL::ComPtr<ID3D12Device2> pDevice);
	ID3D12Device2* GetD3DDevice() { return m_dDevice.Get(); }
	CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
public:
	// ---  GPU RESOURCE CREATION FUNCTIONS -------------------//
	void CreateCommandQueues();
	void CreateFence(Ref<class Fence>& ppFence);
	RenderHandle CreateTexture(Ref<class Texture>& ppTexture, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal = nullptr);
	RenderHandle CreateTexture(Ref<class Texture>& ppTexture, Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState);
	RenderHandle CreateBuffer(Ref<Buffer>& ppBuffer, Uint32 numElements, Uint32 stride, CPVoid initData = nullptr, const std::wstring& debugName = L"Buffer");

	void TransientCommand(std::function<void(CommandContext& cmd)>&& function);
	RenderHandle CreateVertBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
	RenderHandle CreateIndexBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
//	void AddRenderPass(RenderPass* renderPass);

	void BindBuffer(RenderHandle handle, GraphicsContext& context);

	bool IsFenceComplete(Uint64 fenceValue);



	CommandContext& AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type);
	void FreeCommandContext(CommandContext* pContext);
	void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator);

	DescriptorAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, Uint32 count = 1);
	void Shutdown();
	void ReleaseStaleDescriptors();
	void Flush();
private:

	Microsoft::WRL::ComPtr<ID3D12Device2> m_dDevice;

	ContextManager m_ContextManager; // for command list stuff;

	/* ------------------ MANAGERS ------------------ */
	BufferManager m_BufferManager;



	CommandQueue m_GraphicsQueue;
	CommandQueue m_CopyQueue;
	CommandQueue m_ComputeQueue;

	Ref<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

KRS_END_NAMESPACE