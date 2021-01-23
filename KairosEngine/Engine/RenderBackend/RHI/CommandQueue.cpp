#include "krspch.h"
#include "CommandQueue.h"
#include "Fence.h"
#include "RenderDevice.h"
#include "CommandContext.h"

namespace Kairos {
	CommandQueue::CommandQueue(RenderDevice* pDevice, CommandType type)
		: m_Device(pDevice)
		, m_Type(type)
		, m_GPUWaitHandle{ ::CreateEvent(nullptr, false, false, nullptr) }
		, m_AllocatorPool(pDevice, type)
	{
		D3D12_COMMAND_QUEUE_DESC desc = {
			D3DCommandType(type),
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			D3D12_COMMAND_QUEUE_FLAG_NONE,
		};

		auto hr = pDevice->D3DDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_dCommandQueue.ReleaseAndGetAddressOf()));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to Create Command Queue");

		m_Fence = CreateRef<Fence>(m_Device);
	}

	CommandQueue::~CommandQueue()
	{
	}

	void CommandQueue::Shutdown()
	{
		m_AllocatorPool.Shutdown();
		m_dCommandQueue = nullptr;
		m_Fence->Shutdown();
		::CloseHandle(m_GPUWaitHandle);
	}


	void CommandQueue::GetFreeAllocator(ID3D12CommandAllocator** allocator)
	{
		Uint64 completedFence = m_Fence->GetCompletedValue();
		return m_AllocatorPool.RequestAvailableAllocator(completedFence, allocator);
	}

	void CommandQueue::DiscardAllocator(Uint64 fenceValue, ComPtr<ID3D12CommandAllocator> allocator)
	{
		m_AllocatorPool.FreeAllocator(fenceValue, allocator);
	}

	// GPU queue signals the fence once queue finishes executing command lists before signal call
	// returned value is what CPU thread needs to wait for before reusing writable resources
	Uint64 CommandQueue::Signal()
	{
		// GPU internal fence value updated
		Uint64 fenceSignalValue = ++m_CurrFenceValue;
		auto hr = m_dCommandQueue->Signal(m_Fence->D3DFence(), fenceSignalValue);
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
		WaitForGPU(m_CurrFenceValue);
	}

	bool CommandQueue::isFenceComplete(Uint64 fenceValue)
	{
		// completed fence value to regress.
		if (fenceValue > m_CurrFenceValue)
			m_CurrFenceValue = Math::Max(m_CurrFenceValue, m_Fence->GetCompletedValue());

		return fenceValue <= m_CurrFenceValue;
	}

	Uint64 CommandQueue::ExecuteCommands(CommandContext& commandContext)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		ID3D12CommandList* pdCommandList = commandContext.D3DCommandList();

		((ID3D12GraphicsCommandList*)pdCommandList)->Close(); // close the command list to prevent new calls being draw in

		m_dCommandQueue->ExecuteCommandLists(1, &pdCommandList);

		m_dCommandQueue->Signal(m_Fence->D3DFence(), m_CurrFenceValue);
		return m_CurrFenceValue++;
	}


	GraphicsQueue::GraphicsQueue(RenderDevice* pDevice)
		: CommandQueue(pDevice, CommandType::Graphics)
	{}

	GraphicsQueue::~GraphicsQueue()
	{
	}

	ComputeQueue::ComputeQueue(RenderDevice* pDevice)
		: CommandQueue(pDevice, CommandType::Compute)
	{}

	ComputeQueue::~ComputeQueue()
	{
	}

	CopyQueue::CopyQueue(RenderDevice* pDevice)
		: CommandQueue(pDevice, CommandType::Copy)
	{}

	CopyQueue::~CopyQueue()
	{
	}

}
