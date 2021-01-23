#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

class CommandAllocatorPool {
public:
	struct FencedCommandAllocator;
	using AllocatorPool = std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>;
	using FreedAllocators = std::queue<FencedCommandAllocator>;

	CommandAllocatorPool() = default;
	CommandAllocatorPool(RenderDevice* pDevice, CommandType type);
	~CommandAllocatorPool();
	
	void Shutdown();

	void RequestAvailableAllocator(Uint64 fenceValue, ID3D12CommandAllocator** allocator);
	void FreeAllocator(Uint64 fenceValue, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

private:
	struct FencedCommandAllocator {
		Uint64 FenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	};

	RenderDevice* m_Device;
	CommandType m_Type;
	AllocatorPool m_AllocatorPool;
	FreedAllocators m_FreedAllocators;
	std::mutex m_Mutex;

public:

};



KRS_END_NAMESPACE