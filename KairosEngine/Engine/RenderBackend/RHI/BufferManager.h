#pragma once


#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "GraphicsTypes.h"

#include "Buffer.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

struct BufferQueryResult {
	RenderHandle Handle;
	Buffer* AllocBuffer;
};

class BufferManager {
public:
	using AllocBufferList = std::vector<Scope<Buffer>>;
	BufferManager() = default;
	BufferManager(RenderDevice* pDevice, Uint32 size = _64MB);

	void Shutdown();
	
	BufferQueryResult AllocStaticBuffer(Uint32 size, Uint32 stride, void* data = nullptr);
	BufferQueryResult AllocVertBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
	BufferQueryResult AllocIndexBuffer(Uint32 numElements, Uint32 stride, void* data = nullptr);
	BufferQueryResult AllocDynConstBuffer(Uint32 sizeInBytes, Uint32 stride, void* data = nullptr);

	Buffer* GetBuffer(const RenderHandle& handle);


private:
	RenderDevice* m_Device;
	Uint32 m_Size;

	AllocBufferList m_StaticVertBuffers;
	AllocBufferList m_StaticIndexBuffers;
	std::vector<Scope<DynamicBuffer>> m_DynConstBuffers;
	AllocBufferList m_StaticBuffers;

private:
	BufferQueryResult AllocateBuffer(AllocBufferList& list, const struct BufferProperties& props, ResourceState state);

	Uint64 IndexFromHandle(const RenderHandle& handle) const;
	BufferType BufferTypeFromhandle(const RenderHandle& handle) const;


public:
	inline Uint32 Size() const { return m_Size; }

};


KRS_END_NAMESPACE