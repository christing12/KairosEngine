#pragma once
#include "DescriptorHeap.h"
#include "GraphicsTypes.h"
#include "BufferManager.h"
#include "Buffer.h"
#include "CommandQueue.h"
#include "PipelineStateManager.h"
#include "Texture.h"
#include "Shader.h"

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
	friend class SwapChain;
		

	RenderDevice(Microsoft::WRL::ComPtr<ID3D12Device2> pDevice);
	ID3D12Device2* GetD3DDevice() { 
		KRS_CORE_ASSERT(m_dDevice != nullptr, "Render Device is null");
		return m_dDevice.Get(); 
	}
	CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
public:
	// ---  GPU RESOURCE CREATION FUNCTIONS -------------------//
	RenderHandle CreateTexture(D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal = nullptr);
	RenderHandle CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState);
	RenderHandle CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth = 1, Uint32 levels = 1, D3D12_RESOURCE_FLAGS = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	RenderHandle CreateVertBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
	RenderHandle CreateIndexBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);

	RenderHandle CreateGraphicsPSO(const std::string name, std::function<void(GraphicsPipelineProxy&)> fn);
	RenderHandle CreateComputePSO(const std::string name, std::function<void(ComputePiplineProxy&)> fn);
	RenderHandle CreateRootSignature(const std::string name, std::function<void(class RootSignature&)> fn);

	void TransientCommand(std::function<void(CommandContext& cmd)>&& function);

	void CompileAll();

public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(RenderHandle handle, Uint32 index = 0);

	CommandContext& AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type);
	void FreeCommandContext(CommandContext* pContext);
	bool IsFenceComplete(Uint64 fenceValue);


	DescriptorAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType, Uint32 count = 1);
	void Shutdown();
	void ReleaseStaleDescriptors();
	void Flush();
public:
	void CreateFence(Ref<class Fence>& ppFence);
private:
	void CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator);
private:

	Microsoft::WRL::ComPtr<ID3D12Device2> m_dDevice;

	ContextManager m_ContextManager; // for command list stuff;

	/* ------------------ MANAGERS ------------------ */
	BufferManager m_BufferManager;
	TextureManager m_TextureManager;
	ShaderManager m_ShaderManager;
	PipelineStateManager m_PSOManager;

	Scope<CommandQueue> m_GraphicsQueue;
	Scope<CommandQueue> m_CopyQueue;
	Scope<CommandQueue> m_ComputeQueue;

	Ref<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

private:
	void CreateCommandQueues();
};

KRS_END_NAMESPACE