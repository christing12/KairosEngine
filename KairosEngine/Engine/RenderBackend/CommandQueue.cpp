#include "krspch.h"
#include "CommandQueue.h"
#include "Fence.h"
#include "RenderDevice.h"
#include "CommandContext.h"

namespace Kairos {
	CommandQueue::CommandQueue(RenderDevice* pDevice, D3D12_COMMAND_LIST_TYPE commandType, const std::wstring& debugName)
		: m_Device(pDevice)
		, m_CommandListType(commandType)
		, m_GPUWaitHandle{ ::CreateEvent(nullptr, false, false, nullptr) }
		, m_AllocatorPool(pDevice, commandType, debugName)
	{
		D3D12_COMMAND_QUEUE_DESC desc = {
			commandType,
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAG_NONE,
		};

		auto hr = pDevice->GetD3DDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
		m_CommandQueue->SetName(debugName.c_str());
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to Create Command Queue");

		m_Device->CreateFence(m_Fence);
	}

	void CommandQueue::Shutdown()
	{
		m_AllocatorPool.Shutdown();
		m_CommandQueue = nullptr;
		m_Fence->Shutdown();
		::CloseHandle(m_GPUWaitHandle);
	}


	void CommandQueue::GetFreeAllocator(ID3D12CommandAllocator** allocator)
	{
		Uint64 completedFence = m_Fence->GetCompletedValue();
		m_AllocatorPool.GetUnusedAllocator(completedFence, allocator);
	}

	void CommandQueue::DiscardAllocator(Uint64 fenceValue, ComPtr<ID3D12CommandAllocator> allocator)
	{
		m_AllocatorPool.Push(fenceValue, allocator);
	}

	// GPU queue signals the fence once queue finishes executing command lists before signal call
	// returned value is what CPU thread needs to wait for before reusing writable resources
	Uint64 CommandQueue::Signal()
	{
		// GPU internal fence value updated
		Uint64 fenceSignalValue = ++m_FenceValue;
		auto hr = m_CommandQueue->Signal(m_Fence->GetD3DFence(), fenceSignalValue);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with signaling fence");
		return fenceSignalValue;
	}

	/* For When CPU thread needs to wait for GPU with writable resource before reusing it
		Non-Writable resources do not need to wait for GPU thread (only care about writable ones being modified at the same time)
		Checks to see we are still working towards fenceValue
		if we are, set a blocking event (CPU side) to be called once fenceValue is reached
		will only be blocked for duration time
	*/
	void CommandQueue::WaitForGPU(Uint64 fenceValue)
	{
		if (!m_Fence->IsComplete(fenceValue)) { // GetCompletedValue = currently completed fence value (checking if we are still working towards fenceValue)
			m_Fence->SetBlockingEvent(fenceValue, m_GPUWaitHandle);
			::WaitForSingleObject(m_GPUWaitHandle, static_cast<DWORD>(std::chrono::milliseconds::max().count())); // make GPu wait up to the duration time
		}
	}

	/*
		Gives fence a value to let it know to stop at that once all the commands before it have run
		Tell CPU thread to wait for GPU until this fence value has been hit
		this is a blocking method till everything is done before it so all resources are safely released
	*/
	void CommandQueue::Flush()
	{
		Uint64 fenceSignaVal = Signal();
		WaitForGPU(m_FenceValue);
	}

	bool CommandQueue::IsFenceComplete(Uint64 fenceValue)
	{
		// completed fence value to regress.
		if (fenceValue > m_FenceValue)
			m_FenceValue = Math::Max<Uint64>(m_FenceValue, m_Fence->GetCompletedValue());

		return fenceValue <= m_FenceValue;
	}

	Uint64 CommandQueue::ExecuteCommandList(CommandContext* commandContext)
	{
		ID3D12CommandList* pdCommandList = commandContext->GetCommandList();

		std::lock_guard<std::mutex> lockGuard(m_FenceMutex);

		((ID3D12GraphicsCommandList*)pdCommandList)->Close(); // close the command list to prevent new calls being draw in

		m_CommandQueue->ExecuteCommandLists(1, &pdCommandList);

		m_CommandQueue->Signal(m_Fence->GetD3DFence(), m_FenceValue);
		return m_FenceValue++;
	}

}
