#include "krspch.h"
#include "Buffer.h"
#include "RenderDevice.h"

namespace Kairos {
	Buffer::Buffer(RenderDevice* pDevice, const BufferProperties& bufferProps, Kairos::ResourceState state, HeapType heapType)
		: GPUResource{ pDevice, ResourceFormat{bufferProps, state}, heapType}
		, m_Properties{ bufferProps }
	{

	}
	Buffer::Buffer(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> resource, Kairos::ResourceState state)
		: GPUResource{ pDevice, resource, state }
	{
		m_Properties.Offset = 0;
		m_Properties.Size = m_Desc.Width;
		m_Properties.Stride = m_Desc.Width;
	}

	// TODO: Type check to make sure its possible to have a shader view of this buffer?
	Descriptor Buffer::GetSRDescriptor()
	{
		if (m_SRVDescriptor == nullptr) {
			Descriptor srv = m_Device->AllocateDescriptor(DescriptorType::SRV, 1, true).GetDescriptor(0);

			D3D12_SHADER_RESOURCE_VIEW_DESC desc{};

			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Buffer.FirstElement = 0;
			desc.Buffer.NumElements = (UINT)(m_Desc.Width / m_Properties.Stride);
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			desc.Buffer.StructureByteStride = (UINT)m_Properties.Stride;
			desc.Format = m_Desc.Format;

			m_Device->D3DDevice()->CreateShaderResourceView(m_dResource.Get(), &desc, srv.CPUHandle());

			m_SRVDescriptor = CreateScope<Descriptor>(std::move(srv));
		}

		return *m_SRVDescriptor.get();
	}


	DynamicBuffer::DynamicBuffer(RenderDevice* pDevice, ComPtr<ID3D12Resource> resource)
		: Buffer(pDevice, resource, ResourceState::GenericRead)
	{
		KRS_CORE_INFO("Dynamic Buffer being initalized");
		Map();
	}

	DynamicBuffer::DynamicBuffer(RenderDevice* pDevice, const BufferProperties& bufferProps)
		: Buffer(pDevice, bufferProps, ResourceState::GenericRead, HeapType::Upload)
	{
		Map();
	}

	DynamicBuffer* DynamicBuffer::Create(RenderDevice* pDevice, const BufferProperties& props)
	{
		return nullptr;
	}

	DynamicBuffer::~DynamicBuffer()
	{
		Unmap();
	}

	void DynamicBuffer::Map()
	{
		if (m_CPUPtr == nullptr) {
			m_dResource->Map(0, nullptr, &m_CPUPtr);
		}
	}

	void DynamicBuffer::Unmap()
	{
		if (m_CPUPtr != nullptr) {
			m_dResource->Unmap(0, nullptr);
			m_CPUPtr = nullptr;
		}
	}



}

