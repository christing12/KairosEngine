#include "krspch.h"
#include "BufferManager.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "ResourceFormat.h"

namespace Kairos {
	BufferManager::BufferManager(RenderDevice* pDevice, Uint32 size)
		: m_Device(pDevice)
		, m_Size(size)
	{}

	void BufferManager::Shutdown()
	{
		m_StaticVertBuffers.clear();
		m_StaticIndexBuffers.clear();
		m_DynConstBuffers.clear();
	}

	BufferQueryResult BufferManager::AllocStaticBuffer(Uint32 size, Uint32 stride, void* data)
	{
		BufferProperties props{ size, stride };
		BufferQueryResult result = AllocateBuffer(m_StaticBuffers, props, ResourceState::Common);
		if (data != nullptr) {
			m_Device->TransientCommand([&](CommandContext& ctx) {
				ctx.TransitionResource(*result.AllocBuffer, ResourceState::CopyDest, true);
				ctx.CopyBufferRegion(*result.AllocBuffer, 0, data, size);
				ctx.TransitionResource(*result.AllocBuffer, ResourceState::Common);
			});
		}
		return result;
	}

	BufferQueryResult BufferManager::AllocVertBuffer(Uint32 numElements, Uint32 stride, void* data)
	{
		BufferProperties props{ numElements * stride, stride};
		return AllocateBuffer(m_StaticVertBuffers, props, ResourceState::ConstantBuffer);
	}
	BufferQueryResult BufferManager::AllocIndexBuffer(Uint32 numElements, Uint32 stride, void* data)
	{
		BufferProperties props{ numElements * stride, stride};
		return AllocateBuffer(m_StaticIndexBuffers, props, ResourceState::ConstantBuffer);

	}
	BufferQueryResult BufferManager::AllocDynConstBuffer(Uint32 sizeInBytes, Uint32 stride, void* data)
	{
		BufferProperties props{ sizeInBytes, stride };

		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);

		ComPtr<ID3D12Resource> resource;
		auto hr = m_Device->D3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource)
		);

		Scope<DynamicBuffer> buffer = CreateScope<DynamicBuffer>(m_Device, resource);

		Uint32 index = m_DynConstBuffers.size();
		m_DynConstBuffers.emplace_back(std::move(buffer));

		RenderHandle handle{ static_cast<Uint32>(RenderResourceType::Buffer) | (index << 8) };
		return BufferQueryResult{ handle, reinterpret_cast<Buffer*>(m_DynConstBuffers.back().get()) };
	}

	Buffer* BufferManager::GetBuffer(const RenderHandle& handle)
	{
		return nullptr;
	}


	BufferQueryResult BufferManager::AllocateBuffer(AllocBufferList& list, const BufferProperties& props, Kairos::ResourceState state)
	{
		Scope<Buffer> buffer = CreateScope<Buffer>(m_Device, props, state);
		list.emplace_back(std::move(buffer));

		Uint32 index = list.size() - 1;
		RenderHandle handle{ static_cast<Uint32>(RenderResourceType::Buffer) | (index << 8) };
		return BufferQueryResult{ handle, list[index].get() };
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

}