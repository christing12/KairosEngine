#pragma once


#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)



class Descriptor {
public:
	using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
	Descriptor() = default;
	Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, Uint32 index = 0);
	~Descriptor() {}
private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle = { 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle = { 0 };

	Uint32 indexInRange = 0;
public:
	inline D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle() const { return m_CPUHandle; }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle() const { return m_GPUHandle; }
	inline Uint32 Index() const { return indexInRange; }
};

class DescriptorRange {
public:
	using ResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;

	DescriptorRange();
	~DescriptorRange() {}
	DescriptorRange(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
		Uint32 handleSize, Uint32 numDescriptors, Uint32 index = 0);

	Descriptor GetDescriptor(Uint32 offset);

private:
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
	Uint32 m_HandleSize = 32;
	Uint32 m_NumDescriptors = 1;

	Uint32 indexInRange = 0;
public:
	inline D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle() const { return m_CPUHandle; }
	inline D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle() const { return m_GPUHandle; }
	inline Uint32 NumDescriptors() const { return m_NumDescriptors; }
	inline bool isNull() const { return m_CPUHandle.ptr == 0; }
	inline Uint32 Index() const { return indexInRange; }
};






KRS_END_NAMESPACE