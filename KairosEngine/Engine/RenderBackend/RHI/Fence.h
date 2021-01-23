#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

KRS_BEGIN_NAMESPACE(Kairos)

class Fence {
public:
	Fence(class RenderDevice* pDevice);
	~Fence() = default;

	void Shutdown();
	void Reset(Uint64 signalValue);
	void SetBlockingEvent(Uint64 fenceValue, HANDLE blockingEvent);
	bool IsComplete(Uint64 fenceValue);
private:
	Microsoft::WRL::ComPtr<ID3D12Fence> m_dFence;
public:
	inline ID3D12Fence* D3DFence() { return m_dFence.Get(); }
	inline const ID3D12Fence* D3DFence() const { return m_dFence.Get(); }
	Uint64 GetCompletedValue() { return m_dFence->GetCompletedValue(); }
};

KRS_END_NAMESPACE