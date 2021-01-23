#include "krspch.h"
#include "GraphicsTypes.h"

namespace Kairos
{

    D3D12_RESOURCE_STATES D3DResourceStates(ResourceState state)
    {
        D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_COMMON;
        if (EnumMaskContains(state, ResourceState::UnorderedAccess))    states |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (EnumMaskContains(state, ResourceState::PixelAccess))        states |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (EnumMaskContains(state, ResourceState::NonPixelAccess))     states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (EnumMaskContains(state, ResourceState::IndirectArg))        states |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (EnumMaskContains(state, ResourceState::CopyDest))           states |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (EnumMaskContains(state, ResourceState::CopySrc))            states |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (EnumMaskContains(state, ResourceState::GenericRead))        states |= D3D12_RESOURCE_STATE_GENERIC_READ;
        if (EnumMaskContains(state, ResourceState::RenderTarget))       states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (EnumMaskContains(state, ResourceState::ResolveDest))        states |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        if (EnumMaskContains(state, ResourceState::ResolveSrc))         states |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        if (EnumMaskContains(state, ResourceState::Present))            states |= D3D12_RESOURCE_STATE_PRESENT;
        if (EnumMaskContains(state, ResourceState::DepthRead))          states |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (EnumMaskContains(state, ResourceState::DepthWrite))         states |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (EnumMaskContains(state, ResourceState::ConstantBuffer))     states |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        return states;
    }


    D3D12_DESCRIPTOR_HEAP_TYPE D3DDescriptorHeap(DescriptorType type)
    {
        switch (type)
        {
        case DescriptorType::RTV: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        case DescriptorType::DSV: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        case DescriptorType::Sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        case DescriptorType::CBV:
        case DescriptorType::SRV:
        case DescriptorType::UAV:
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        default:
            KRS_CORE_ERROR("Not a valid Descriptor Type to convert");
        }
    }


    D3D12_COMMAND_LIST_TYPE D3DCommandType(CommandType type)
    {
        switch (type)
        {
        case CommandType::Copy: return D3D12_COMMAND_LIST_TYPE_COPY;
        case CommandType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    D3D12_HEAP_TYPE D3DHeapType(HeapType type)
    {
        switch (type)
        {
        case HeapType::Custom: return D3D12_HEAP_TYPE_CUSTOM;
        case HeapType::Default: return D3D12_HEAP_TYPE_DEFAULT;
        case HeapType::Readback: return D3D12_HEAP_TYPE_READBACK;
        case HeapType::Upload: return D3D12_HEAP_TYPE_UPLOAD;

        }
    }

}