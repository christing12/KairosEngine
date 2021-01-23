#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include "MeshComponent.h"
#include "Editor/EditorCamera.h"
#include "Core/Events/Event.h"

KRS_BEGIN_NAMESPACE(Kairos)

class Material;
class Mesh;
class Texture;
class Buffer;
class MeshComponent;
class RenderDevice;
class CommandContext;

struct GPUMaterialEntry
{
	Uint32 AlbedoMapIndex;
	Uint32 NormalMapIndex;
	Uint32 RoughnessMapIndex;
	Uint32 MetalnessMapIndex;
};

class Scene {
public:
	Scene() = default;
	~Scene() {}

	void Init(RenderDevice* pDevice);
	void Update(float deltaTime);

	MeshComponent& AddMeshInstance(MeshComponent&& meshInstance);
	Mesh& AddMesh(Mesh&& mesh);
	// moves
	Material& AddMaterial(Material&& mat);
	Light& AddLight(Light&& light);
	bool OnEvent(Event& e);

	void Bind(CommandContext& context);

	void UploadMaterials();
	void UploadMeshes();
	void UploadLights();


	// TODO: move this to its own class 
	Material* LoadMaterial(const std::string& name);
private:
	EditorCamera mCamera;

	RenderDevice* m_Device;

	std::vector<Mesh> m_CachedMeshes;
	std::vector<MeshComponent> m_MeshInstances;
	std::vector<Material> m_Materials;
	std::vector<Light> m_Lights;

	Buffer* m_LightTableBuffer;
	Buffer* m_MatTableBuffer;
	Buffer* m_MergedVertexBuffer;
	Buffer* m_MergedIndexBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_VTest;
	D3D12_INDEX_BUFFER_VIEW m_ITest;
public:
	inline EditorCamera& GetCamera() { return mCamera; }
	inline std::vector<Mesh>& MergedMeshes() { return m_CachedMeshes; }
	inline Buffer* MergeIndices() { return m_MergedVertexBuffer; }

};


KRS_END_NAMESPACE