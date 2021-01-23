#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include "GraphicsTypes.h"
#include "CommandAllocatorPool.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class Fence;
class CommandContext;

class CommandQueue {
public:
	CommandQueue() = default;
	
	CommandQueue(RenderDevice* pDevice, CommandType type);
	~CommandQueue();
	void Shutdown();
	void GetFreeAllocator(ID3D12CommandAllocator** allocator);
	void DiscardAllocator(Uint64 fenceValue, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

	Uint64 Signal();
	void WaitForGPU(Uint64 fenceValue);
	void Flush();
	bool isFenceComplete(Uint64 fenceValue);

	Uint64 ExecuteCommands(class CommandContext& context);

protected:
	RenderDevice* m_Device;
	CommandType m_Type;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_dCommandQueue;

	// fence wrapper that is associated with this queue
	Ref<Fence> m_Fence;

	// blocking event used to stall CPU to wait for GPU
	HANDLE m_GPUWaitHandle;

	// the most recent fence value issued by this command queue
	Uint64 m_CurrFenceValue = 0;

	CommandAllocatorPool m_AllocatorPool;
	std::mutex m_Mutex;

public:
	inline ID3D12CommandQueue* D3DQueue() { return m_dCommandQueue.Get(); }
	inline const ID3D12CommandQueue* D3DQueue() const { return m_dCommandQueue.Get(); }
};


class GraphicsQueue : public CommandQueue {
public:
	GraphicsQueue() = default;
	GraphicsQueue(RenderDevice* pDevice);
	~GraphicsQueue();
private:
};

class ComputeQueue : public CommandQueue {
public:
	ComputeQueue() = default;
	ComputeQueue(RenderDevice* pDevice);
	~ComputeQueue();
private:
};

class CopyQueue : public CommandQueue {
public:
	CopyQueue() = default;
	CopyQueue(RenderDevice* pDevice);
	~CopyQueue();
private:
};




KRS_END_NAMESPACE