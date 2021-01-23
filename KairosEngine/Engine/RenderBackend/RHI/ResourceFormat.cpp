#include "krspch.h"
#include "ResourceFormat.h"

namespace Kairos {

    ResourceFormat::ResourceFormat(const BufferProperties& bufferProps, Kairos::ResourceState state)
    {
        ZeroMemory(&m_Desc, sizeof(m_Desc));
        m_Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        m_Desc.Width = bufferProps.Size;
        m_Desc.Height = 1;
        m_Desc.DepthOrArraySize = 1;
        m_Desc.MipLevels = 1;
        m_Desc.Format = DXGI_FORMAT_UNKNOWN;
        m_Desc.SampleDesc.Count = 1;
        m_Desc.SampleDesc.Quality = 0;
        m_Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;


        m_InitState = state;
    }

    ResourceFormat::ResourceFormat(const TextureProperties& textureProps, Kairos::ResourceState state)
    {
      

        ZeroMemory(&m_Desc, sizeof(m_Desc));



        switch (textureProps.Type)
        {
        case TextureType::Tex1D: m_Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D; break;
        case TextureType::Tex2D: m_Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; break;
        case TextureType::Tex3D: m_Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D; break;
        }

        m_Desc.Width = textureProps.Width;
        m_Desc.Height = textureProps.Height;
        m_Desc.DepthOrArraySize = textureProps.Depth;
        m_Desc.MipLevels = textureProps.MipLevels;
        m_Desc.SampleDesc.Count = 1;
        m_Desc.SampleDesc.Quality = 0;
        m_Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        //m_Desc.Alignment = 
        m_Desc.Format = textureProps.Format;

        m_InitState = state;
        DetermineFlagUsage(m_InitState);
    }

    void ResourceFormat::DetermineFlagUsage(Kairos::ResourceState state)
    {
        m_Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (EnumMaskEquals(state, ResourceState::RenderTarget))
        {
            m_Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }

        if (EnumMaskEquals(state, ResourceState::DepthRead) ||
            EnumMaskEquals(state, ResourceState::DepthWrite))
        {
            m_Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            if (!EnumMaskEquals(state, ResourceState::PixelAccess) &&
                !EnumMaskEquals(state, ResourceState::NonPixelAccess))
            {
                m_Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
            }
        }

        if (EnumMaskEquals(state, ResourceState::UnorderedAccess))
        {
            m_Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
    }

}
