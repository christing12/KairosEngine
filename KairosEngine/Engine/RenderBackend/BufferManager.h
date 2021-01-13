#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>

#include "GraphicsTypes.h"
#include "Buffer.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

struct StaticBufferAllocation {
	D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;

	Uint32 OffsetFromStart;
	Microsoft::WRL::ComPtr<ID3D12Resource> NativeResource;
};

// Video Memory resources (only GPU visible, CPU cannot write to them)
class StaticBufferHeap {
public:
	

	friend class BufferManager;
	KRS_CLASS_DEFAULT(StaticBufferHeap);

	StaticBufferHeap(RenderDevice* pDevice, BufferType type, Uint32 size);


	StaticBufferAllocation Allocate(Uint32 sizeInBytes, Uint32 alignment = 256u);
private:
	GPUResource m_BaseResource;
	GPUResource m_UploadBuffer;

	RenderDevice* m_Device;
	BufferType m_Type;
	Uint32 m_Size;
	Uint32 m_Offset;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_VBView;
	std::vector<D3D12_INDEX_BUFFER_VIEW> m_IBView;
private:
	bool HasSpace(Uint32 size, Uint32 alignment);
};


class BufferManager {
public:
	KRS_CLASS_DEFAULT(BufferManager);
	BufferManager(RenderDevice* pDevice, Uint32 size = _64MB);

	Uint32 Size() const { return MEMORY_SIZE; }
	// only use internally! do not keep copies!!!!
	Buffer& GetUnreferencedBuffer(RenderHandle handle);
	void AllocVertBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
	void AllocIndexBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
private:
	
	void Init();

private:


	void AllocBuffer(StaticBufferHeap& heap, Uint32 numElements, Uint32 stride, void* data);

	RenderDevice* m_Device;

	const Uint32 MEMORY_SIZE = 0;

	// for one time uploads -> GPU at setup time
	StaticBufferHeap m_BufferHeaps[BufferType::NumTypes];
private:
};


KRS_END_NAMESPACE