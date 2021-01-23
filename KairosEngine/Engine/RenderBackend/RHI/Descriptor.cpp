#include "krspch.h"
#include "Descriptor.h"

namespace Kairos {
	Descriptor::Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, Uint32 index)
		: m_CPUHandle(cpuHandle)
		, m_GPUHandle(gpuHandle)
		, indexInRange(index)
	{}

	Descriptor::~Descriptor()
	{
	}

	DescriptorRange::DescriptorRange()
		: m_CPUHandle{ 0 }
		, m_GPUHandle{ 0 }
	{
	}

	DescriptorRange::DescriptorRange(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, Uint32 handleSize, Uint32 numDescriptors, Uint32 index)
		: m_CPUHandle(cpuHandle)
		, m_GPUHandle(gpuHandle)
		, m_HandleSize(handleSize)
		, m_NumDescriptors(numDescriptors)
		, indexInRange(index)

	{}

	Descriptor DescriptorRange::GetDescriptor(Uint32 offset)
	{
		KRS_CORE_ASSERT(offset < m_NumDescriptors, "Offset is outside of this allocated descriptor range");

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = { m_CPUHandle.ptr + (m_HandleSize * offset) };
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = { m_GPUHandle.ptr + (m_HandleSize * offset) };

		return Descriptor{ cpuHandle, gpuHandle, indexInRange + offset };
	}



}

