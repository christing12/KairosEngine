#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)

enum class TextureType : Uint8
{
	Tex1D,
	Tex2D,
	Tex3D
};

struct TextureProperties {
	Uint32 Width			= 1;
	Uint32 Height			= 1;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: change to custom type
	TextureType Type = TextureType::Tex2D;
	Uint32 Depth			= 1;
	Uint32 MipLevels		= 1;

	TextureProperties() = default;
	TextureProperties(Uint32 _width, Uint32 _height, DXGI_FORMAT format, TextureType type = TextureType::Tex2D, Uint32 _depth = 1, Uint32 _mipLevels = 1)
		: Width(_width)
		, Height(_height)
		, Format(format)
		, Type(type)
		, Depth(_depth)
		, MipLevels(_mipLevels)
	{}
};

struct BufferProperties {
	Uint32 Size;
	Uint32 Stride;
	Uint32 Offset = 0; // from the start of the resource

	BufferProperties() = default;
	BufferProperties(Uint32 size, Uint32 stride, Uint32 offset = 0)
		: Size(size)
		, Stride(stride)
		, Offset(offset)
	{}
};

class ResourceFormat {
public:
	ResourceFormat() = default;
	ResourceFormat(const BufferProperties& bufferProps, ResourceState state = ResourceState::Common);
	ResourceFormat(const TextureProperties& textureProps, ResourceState state = ResourceState::Common);

private:
	D3D12_RESOURCE_DESC m_Desc;
	ResourceState m_InitState;
	ResourceState m_ExpecteState;
private:
	void DetermineFlagUsage(ResourceState state);
public:
	inline const D3D12_RESOURCE_DESC& GetDesc() const { return m_Desc; }
	inline ResourceState ResourceState() const { return m_InitState; }
};

KRS_END_NAMESPACE