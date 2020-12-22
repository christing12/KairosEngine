#pragma once


#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"

#include <queue>

KRS_BEGIN_NAMESPACE(Kairos)

class CommandAllocatorPool {
public:

	CommandAllocatorPool(class RenderDevice* pDevice, D3D12_COMMAND_LIST_TYPE type, const std::wstring& name = L"Command Allocator");
	~CommandAllocatorPool();

	void Shutdown();

	void GetUnusedAllocator(Uint64 fenceValue, ID3D12CommandAllocator** allocator);
	void Push(Uint64 fenceValue, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);
private:
	// replacement for std::pair to make it clearer whats actually going on
	struct CommandAllocatorEntry {
		Uint64 AssociatedFenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	};
	typedef struct CommandAllocatorEntry CommandAllocatorEntry;

	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_Allocators;
	std::queue<CommandAllocatorEntry> m_CommandAllocators;

	D3D12_COMMAND_LIST_TYPE m_CommandType;
	class RenderDevice* m_Device; // dont need strong ref I think (maybe change)

	std::mutex m_AllocationMutex;

	//debug
	std::wstring m_Name = L"Command Allocator Pool";
};

KRS_END_NAMESPACE