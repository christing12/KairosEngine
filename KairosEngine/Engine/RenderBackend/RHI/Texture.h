#pragma once

#include "GPUResource.h"
#include "ResourceFormat.h"
#include "Descriptor.h"
#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class Descriptor;
class DescriptorRange;


class Texture : public GPUResource {
public:
	Texture() = default;
	Texture(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource, Kairos::ResourceState state);
	Texture(RenderDevice* pDevice, const struct TextureProperties& bufferProps, Kairos::ResourceState state = ResourceState::Common);

	Descriptor GetSRDescriptor();
	Descriptor GetDSVDescriptor();
	
	Descriptor GetRTVDescriptor(Uint32 offset = 0);
	Descriptor GetUAVDescriptor(Uint32 offset = 0);
protected:
	TextureProperties m_Properties;

	Scope<DescriptorRange> m_RTVDescriptor;
	Scope<Descriptor> m_DSVDescriptor;
	Scope<Descriptor> m_SRVDescriptor;
	Scope<DescriptorRange> m_UAVDescriptors;

	static D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const Texture* texture, Uint32 mipSlice, Uint32 arraySlice = 0, Uint32 planeSlice = 0);
public:
	inline Uint32 Width() const { return m_Properties.Width; }
	inline Uint32 Height() const { return m_Properties.Height; }
	inline Uint32 Depth() const { return m_Properties.Depth; }
	inline Uint32 MipLevels() const { return m_Properties.MipLevels; }
};


KRS_END_NAMESPACE
