#pragma once

#include <Core/BaseTypes.h>

#include <wrl.h>

KRS_BEGIN_NAMESPACE(Kairos)

class Fence {
public:
	Fence(class RenderDevice* pDevice);
	~Fence() = default;

	void Shutdown();

	void Reset(Uint64 signalValue);
	ID3D12Fence* GetD3DFence() { return m_dFence.Get(); }
	Uint64 GetCompletedValue() { return m_dFence->GetCompletedValue(); }

	void SetBlockingEvent(Uint64 fenceValue, HANDLE blockingEvent);
	bool IsComplete(Uint64 FenceValueToQuery);
private:
	Microsoft::WRL::ComPtr<ID3D12Fence> m_dFence;
};

KRS_END_NAMESPACE