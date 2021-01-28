#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

class RenderDevice;
class Scene;
class Buffer;

KRS_BEGIN_NAMESPACE(Kairos)

struct GPUMaterialEntry
{
	Uint32 AlbedoMapIndex;
	Uint32 NormalMapIndex;
	Uint32 RoughnessMapIndex;
	Uint32 MetalnessMapIndex;
};

class SceneGPUStorage {
public:
	SceneGPUStorage() = default;
	SceneGPUStorage(RenderDevice* pDevice, Scene* scene);


	void UploadMaterials();
	void UploadMeshes();
	void UploadLights();

private:
	RenderDevice* m_Device;
	Scene* m_Scene;

	// TODO: Change these all to unique pointers later

	Buffer* m_MergedVertexBuffer;
	Buffer* m_MergedIndexBuffer;
	Buffer* m_MatTableBuffer;
	Buffer* m_LightTableBuffer;
public:
	inline Buffer* MergedVertices() { return m_MergedVertexBuffer; }
	inline Buffer* MergedIndices() { return m_MergedIndexBuffer; }
	inline Buffer* MatTableBuffer() { return m_MatTableBuffer; }
	inline Buffer* LightTableBuffer() { return m_LightTableBuffer; }

};


KRS_END_NAMESPACE