#pragma once

#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>
#include <Graphics/Core/Resource.h>
#include <Graphics/Core/DescriptorHeap.h>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

class Buffer : public Resource {
public:
	Buffer(RenderDevice* pDevice, Uint32 numElements, Uint32 stride, CPVoid initData = nullptr, const std::wstring& debugName = L"Buffer");

	Uint32 GetBufferSize() const { return m_BufferSize; }

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t offset, Uint32 size, Uint32 stride) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t offset, Uint32 size, bool b32bit) const;
	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstanBufferView(Uint32 offset, Uint32 size) const;
protected:
	Uint32 m_BufferSize = 0;
	CPVoid m_Data = nullptr;
	Uint32 m_NumElements = 0;
	Uint32 m_Stride = 0;

	D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
};

class DynamicUploadBuffer : Resource {
public:
	DynamicUploadBuffer(RenderDevice* pDevice, Uint32 numElements, Uint32 stride, const std::wstring& debugName = L"Dynamic Upload Buffer");
	DynamicUploadBuffer(RenderDevice* pDevice, Uint32 bufferSize, const std::wstring& debugName = L"Dynamic Upload Buffer");
	virtual ~DynamicUploadBuffer();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCBV(Uint32 size, Uint32 offset = 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCBVHandle() const { return m_CBVAllocation.GetDescriptorHandle(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return m_GPUAddress; }
	void SetDynamicData(const void* data, Uint32 size, Uint32 offset = 0);

	void* Map();
	void Unmap();
private:
	D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;
	void* m_CPUAddress = nullptr;
	Uint32 m_NumElements;
	Uint32 m_Stride;
	Uint32 m_BufferSize;

	DescriptorAllocation m_CBVAllocation;

	void Create(const std::wstring& debugName);

};


KRS_END_NAMESPACE