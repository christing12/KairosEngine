#include "krspch.h"
#include "Fence.h"
#include "RenderDevice.h"

namespace Kairos {
	Fence::Fence(RenderDevice* pDevice)
		: m_dFence(nullptr)
	{
		auto hr = pDevice->D3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_dFence));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to create D3D Fence");
	}
	void Fence::Shutdown()
	{
		m_dFence = nullptr;
	}

	void Fence::Reset(Uint64 signalValue)
	{
		m_dFence->Signal(signalValue);
	}

	// checks to see if the queried fence value is in the past (lower than currently completed fence value)
	bool Fence::IsComplete(Uint64 FenceValueToQuery)
	{
		return FenceValueToQuery < m_dFence->GetCompletedValue();
	}

	// blocks execution on CPU thread until fence value has been reached
	void Fence::SetBlockingEvent(Uint64 fenceValue, HANDLE blockingEvent) {
		m_dFence->SetEventOnCompletion(fenceValue, blockingEvent);
	}

}