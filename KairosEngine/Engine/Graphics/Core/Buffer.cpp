#include "krspch.h"
#include "Buffer.h"
#include "RenderDevice.h"
#include "CommandContext.h"

namespace Kairos {
    Buffer::Buffer(RenderDevice* pDevice, Uint32 numElements, Uint32 stride, CPVoid initData, const std::wstring& debugName)
        : Resource(pDevice)
        , m_NumElements(numElements)
        , m_Stride(stride)
        , m_BufferSize(numElements* stride)
        , m_Data(initData)
        , m_GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(0))
    {
        KRS_CORE_ASSERT(pDevice != nullptr, "Issue with redner device");

        m_Desc = Resource::CreateBufferDesc(m_BufferSize);

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        bool initBuffer = m_Data != nullptr && m_BufferSize > 0;
        m_CurrState = initBuffer ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON;

        // creates resource and implicit heap to contain entire resource -> resource mapped to heap
        /* https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommittedresource */
        auto hr = m_Device->GetD3DDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &m_Desc,
            m_CurrState,
            nullptr,
            IID_PPV_ARGS(&m_Resource)
        );

        KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to create ID3D12Resource for buffer");

        m_GPUAddress = m_Resource->GetGPUVirtualAddress();

        CheckFeatureSupport();

        if (initBuffer) {
            CommandContext::InitBuffer(m_Device, *this, m_Data, m_BufferSize);
        }
    }


    D3D12_VERTEX_BUFFER_VIEW Buffer::VertexBufferView(size_t offset, Uint32 size, Uint32 stride) const
    {
        D3D12_VERTEX_BUFFER_VIEW VBView;
        VBView.BufferLocation = m_GPUAddress + offset;
        VBView.SizeInBytes = size;
        VBView.StrideInBytes = stride;

        return VBView;
    }

    D3D12_INDEX_BUFFER_VIEW Buffer::IndexBufferView(size_t offset, Uint32 size, bool b32bit) const
    {
        D3D12_INDEX_BUFFER_VIEW IBView;
        IBView.BufferLocation = m_Resource->GetGPUVirtualAddress() + offset;
        IBView.Format = b32bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        IBView.SizeInBytes = size;
        return IBView;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Buffer::CreateConstanBufferView(Uint32 offset, Uint32 size) const
    {
        KRS_CORE_INFO(offset + size);
        KRS_CORE_ASSERT(offset + size <= m_BufferSize, "TOO BIG VIEW");
        size = Math::AlignUp(size, 256);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvdesc;
        cbvdesc.BufferLocation = m_GPUAddress + (size_t)offset;
        cbvdesc.SizeInBytes = size;

        D3D12_CPU_DESCRIPTOR_HANDLE cbv = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1).GetDescriptorHandle();

        m_Device->GetD3DDevice()->CreateConstantBufferView(&cbvdesc, cbv);
        return cbv;
    }

    DynamicUploadBuffer::DynamicUploadBuffer(RenderDevice* pDevice, Uint32 numElements, Uint32 stride, const std::wstring& debugName)
        : Resource(pDevice)
        , m_NumElements(numElements)
        , m_Stride(stride)
        , m_BufferSize(numElements* stride)
        , m_CPUAddress(nullptr)
        , m_GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(0))
    {
        Create(debugName);
    }
    DynamicUploadBuffer::DynamicUploadBuffer(RenderDevice* pDevice, Uint32 bufferSize, const std::wstring& debugName)
        : Resource(pDevice)
        , m_BufferSize(bufferSize)
        , m_CPUAddress(nullptr)
        , m_GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
    {
        Create(debugName);
    }

    DynamicUploadBuffer::~DynamicUploadBuffer()
    {
        if (m_CPUAddress != nullptr)
            Unmap();
        m_Resource = nullptr;
        m_GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;

    }
    D3D12_CPU_DESCRIPTOR_HANDLE DynamicUploadBuffer::GetCBV(Uint32 size, Uint32 offset)
    {
        size = Math::AlignUp(size, 256);

        D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
        CBVDesc.BufferLocation = m_GPUAddress + size_t(offset);
        CBVDesc.SizeInBytes = size;

        m_CBVAllocation = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_Device->GetD3DDevice()->CreateConstantBufferView(&CBVDesc, m_CBVAllocation.GetDescriptorHandle());
        return m_CBVAllocation.GetDescriptorHandle();
    }

    void DynamicUploadBuffer::SetDynamicData(const void* data, Uint32 size, Uint32 offset)
    {
        //   KRS_CORE_ASSERT(size <= m_BufferSize, "UNABLE TO SET DATA");
        memcpy(reinterpret_cast<Uint8*>(m_CPUAddress) + size_t(offset), data, size);
    }

    void* DynamicUploadBuffer::Map()
    {
        KRS_CORE_ASSERT(m_CPUAddress == nullptr, "Dynamic Upload Buffer has already been mapped");
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = 0;

        m_Resource->Map(0, &range, &m_CPUAddress);
        return m_CPUAddress;
    }
    void DynamicUploadBuffer::Unmap()
    {
        KRS_CORE_ASSERT(m_CPUAddress != nullptr, "Dynamic Upload Buffer is not mapped already");
        m_Resource->Unmap(0, nullptr);
        m_CPUAddress = nullptr;
    }
    void DynamicUploadBuffer::Create(const std::wstring& debugName)
    {
        m_Desc = Resource::CreateBufferDesc(1024 * 64);

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        auto hr = m_Device->GetD3DDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &m_Desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_Resource)
        );
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with Creating GPU Resource for Dynamic Upload Buffer");
        m_Resource->SetName(debugName.c_str());

        m_GPUAddress = m_Resource->GetGPUVirtualAddress();
        m_CPUAddress = nullptr;
    }
}