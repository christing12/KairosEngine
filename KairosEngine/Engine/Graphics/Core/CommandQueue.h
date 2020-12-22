#pragma once
#include "CommandAllocatorPool.h"


#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"


namespace Kairos {
	class CommandQueue {
	public:
		CommandQueue(Ref<class RenderDevice> pDevice, D3D12_COMMAND_LIST_TYPE commandType, const std::wstring& debugName = L"Command Queue");
		~CommandQueue();


		void Shutdown();

		ID3D12CommandQueue* GetD3DCommandQueue() { return m_CommandQueue.Get(); }
		void GetFreeAllocator(ID3D12CommandAllocator** allocator);
		void DiscardAllocator(Uint64 fenceValue, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

		Uint64 Signal(); // fence value is updated on GPU with this
		void WaitForGPU(Uint64 fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max());
		void Flush();

		Uint64 ExecuteCommandList(class CommandContext* commandList);
	private:
		Ref<class RenderDevice> m_Device; // strong reference to device-> may just need weak reference
		D3D12_COMMAND_LIST_TYPE m_CommandListType;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Ref<class Fence> m_Fence;
		HANDLE m_GPUWaitHandle;
		Uint64 m_FenceValue = 0;

		CommandAllocatorPool m_AllocatorPool;

		std::mutex m_FenceMutex;
	};
}