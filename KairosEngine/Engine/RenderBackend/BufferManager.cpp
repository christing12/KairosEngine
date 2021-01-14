#include "krspch.h"
#include "BufferManager.h"
#include "RenderDevice.h"
#include "CommandContext.h"
namespace Kairos {
	static D3D12_RESOURCE_STATES GetResourceTransitionState(BufferType type)
	{
		D3D12_RESOURCE_STATES s;
		switch (type)
		{
		case BufferType::Constant: s = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
		case BufferType::Vertex: s = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
		case BufferType::Index: s = D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
		default:
			KRS_CORE_ERROR("Unknown Buffer Type: No Resource Trasition state detected");
			break;
		}
		return s;
	}


	BufferManager::BufferManager(RenderDevice* pDevice, Uint32 size)
		: m_Device(pDevice)
		
	{

	}

	Buffer& BufferManager::GetUnreferencedBuffer(RenderHandle handle)
	{
		return Buffer{};
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Buffer, "Not the right type");
		Uint64 index = IndexFromHandle(handle);
		BufferType type = BufferTypeFromhandle(handle);

		//m_BufferHeaps[type].

		//return &m_AllocatedTextures[index];
	}

	RenderHandle BufferManager::AllocVertBuffer(Uint32 numElements, Uint32 stride, void* data)
	{

		StaticBufferAllocation alloc = m_BufferHeaps[BufferType::Vertex].Allocate(numElements * stride);
		Buffer vertBuffer;
		vertBuffer.Data = data;
		vertBuffer.GPUAddress = alloc.GPUAddress;
		vertBuffer.NumElements = numElements;
		vertBuffer.Stride = stride;
		vertBuffer.Offset = alloc.OffsetFromStart;
		vertBuffer.ResourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		vertBuffer.NativeResource = alloc.NativeResource;



		Uint32 index = m_AllocatedVertBuffers.size();
		m_AllocatedVertBuffers.push_back(vertBuffer);

		RenderHandle handle{ static_cast<Uint32>(RenderResourceType::Buffer) | (index << 8) };
		return handle;
		
	}

	RenderHandle BufferManager::AllocIndexBuffer(Uint32 numElements, Uint32 stride, void* data)
	{
		StaticBufferAllocation alloc = m_BufferHeaps[BufferType::Index].Allocate(numElements * stride);
		Buffer idxBuffer;
		idxBuffer.Data = data;
		idxBuffer.GPUAddress = alloc.GPUAddress;
		idxBuffer.NumElements = numElements;
		idxBuffer.Stride = stride;
		idxBuffer.Offset = alloc.OffsetFromStart;
		idxBuffer.ResourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		idxBuffer.NativeResource = alloc.NativeResource;

		Uint32 index = m_AllocatedIndexBuffers.size();
		m_AllocatedIndexBuffers.push_back(idxBuffer);

		RenderHandle handle{ static_cast<Uint32>(RenderResourceType::Buffer) | (index << 8) };
		return handle;
	}


	void BufferManager::Init() {
		m_BufferHeaps[BufferType::Vertex] = StaticBufferHeap{ m_Device, BufferType::Vertex, MEMORY_SIZE };
		m_BufferHeaps[BufferType::Index] = StaticBufferHeap{ m_Device, BufferType::Index, MEMORY_SIZE };
	}

	Uint64 BufferManager::IndexFromHandle(const RenderHandle& handle) const
	{
		constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 8) - 1);
		return Uint64((handle.handle & resourceMask) >> 8);
	}

	BufferType BufferManager::BufferTypeFromhandle(const RenderHandle& handle) const
	{
		constexpr Uint64 bufferTypeMask = ((1 << 32) - 1) & ~((1 << 24) - 1);
		return BufferType((handle.handle & bufferTypeMask) >> 24);
	}


	void BufferManager::AllocBuffer(StaticBufferHeap& heap, Uint32 numElements, Uint32 stride, void* data)
	{
		StaticBufferAllocation alloc = heap.Allocate(numElements * stride, 256);
		
		
	}


	StaticBufferHeap::StaticBufferHeap(RenderDevice* pDevice, BufferType type, Uint32 size)
		: m_Device(pDevice)
		, m_Type(type)
		, m_Size(size)
		, m_Offset(0)
	{

		// create D3D Resource
		auto hr = m_Device->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_Size),
			GetResourceTransitionState(m_Type),
			nullptr,
			IID_PPV_ARGS(&m_BaseResource.NativeResource)
		);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Problem with creating static buffer heap");
	
		m_BaseResource.GPUAddress = m_BaseResource.NativeResource->GetGPUVirtualAddress();
		m_BaseResource.ResourceState = GetResourceTransitionState(m_Type);

		// create D3D Resource
		hr = m_Device->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_Size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_BaseResource.NativeResource)
		);
	}

	StaticBufferAllocation StaticBufferHeap::Allocate(Uint32 sizeInBytes, Uint32 alignment)
	{
		KRS_CORE_ASSERT(HasSpace(sizeInBytes, alignment), "No Space in Static Heap (GPU VISIBLE)");


		Uint32 alignedSize = Math::AlignUp(sizeInBytes, alignment);
		m_Offset = Math::AlignUp(m_Offset, alignment);

		StaticBufferAllocation alloc;
		alloc.GPUAddress = m_BaseResource.GPUAddress + m_Offset;
		alloc.NativeResource = m_BaseResource.NativeResource;
		alloc.OffsetFromStart = m_Offset;

		m_Offset += alignedSize;

		return alloc;
	}

	bool StaticBufferHeap::HasSpace(Uint32 size, Uint32 alignment)
	{
		Uint32 alignedSize = Math::AlignUp(size, alignment);
		Uint32 alignedOffset = Math::AlignUp(m_Offset, alignment);
		return (alignedSize + alignedOffset) <= m_Size;
	}



}

