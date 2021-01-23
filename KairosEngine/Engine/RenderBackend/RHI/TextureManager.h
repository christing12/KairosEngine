#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class Texture;
struct TextureProperties;

struct TextureQueryResult {
	RenderHandle Handle;
	Texture* Texture;
};

class TextureManager {
public:
	TextureManager() = default;
	TextureManager(RenderDevice* pDevice);
	~TextureManager();


	void Shutdown();

	TextureQueryResult CreateTexture(const TextureProperties& desc, ResourceState state = ResourceState::Common);
	TextureQueryResult CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource, ResourceState state);
	TextureQueryResult CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth = 1, Uint32 levels = 1);


	Texture* GetUnreferencedTexture(RenderHandle handle);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(RenderHandle handle, Uint32 index = 0);
private:
	RenderDevice* m_Device;
	std::vector<Scope<Texture>> m_AllocatedTextures;
	void InitTextureViews(Texture* texture);
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const Texture* texture, Uint32 mipSlice, Uint32 arraySlice = 0, Uint32 planeSlice = 0);
};


KRS_END_NAMESPACE