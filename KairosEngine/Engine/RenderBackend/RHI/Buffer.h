#pragma once

#include "GPUResource.h"
#include "ResourceFormat.h"
#include "Descriptor.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;


class Buffer : public GPUResource {
public:
	Buffer() = default;
	Buffer(RenderDevice* pDevice, const struct BufferProperties& bufferProps, Kairos::ResourceState state);
	Buffer(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> resource, Kairos::ResourceState state);
	~Buffer() = default;

	Descriptor GetSRDescriptor();

protected:
	BufferProperties m_Properties;

	Scope<Descriptor> m_SRVDescriptor;
	Scope<DescriptorRange> m_UAVDescriptors;

public:
	inline Uint32 Offset() const { return m_Properties.Offset; }
	inline Uint32 Size() const { return m_Properties.Size; }
	inline Uint32 Stride() const { return m_Properties.Stride; }
	inline Uint32 NumElements() const { return m_Properties.Size / m_Properties.Stride; }
};

// buffer only exists on the GPU
class StaticBuffer : public GPUResource {
public:

private:
};


// upload buffer (can be CBV, SRV, UAV...)
class DynamicBuffer : public Buffer {
public:
	DynamicBuffer() = default;
	// no resource state -> has to start as an ResourceState::GenericRead
	DynamicBuffer(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> resource);
	~DynamicBuffer();
	void Map();
	void Unmap();

private:
	void* m_CPUPtr = nullptr;

public:
	inline void* DataPtr() { return m_CPUPtr; }
};

KRS_END_NAMESPACE