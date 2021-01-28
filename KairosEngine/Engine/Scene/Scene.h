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
class SceneGPUStorage;



class Scene {
public:
	friend class SceneGPUStorage;
	Scene() = default;
	~Scene() = default;

	void Init(RenderDevice* pDevice);
	void Update(float deltaTime);

	MeshComponent& AddMeshInstance(MeshComponent&& meshInstance);
	Mesh& AddMesh(Mesh&& mesh);
	Material& AddMaterial(Material&& mat);
	Light& AddLight(Light&& light);


	bool OnEvent(Event& e);

	// TODO: move this to its own class 
	Material* LoadMaterial(const std::string& name);
private:
	RenderDevice* m_Device;
	Scope<SceneGPUStorage> m_GPUStorage;
	EditorCamera mCamera;

	std::vector<Mesh> m_CachedMeshes;
	std::vector<MeshComponent> m_MeshInstances;
	std::vector<Material> m_Materials;
	std::vector<Light> m_Lights;
public:
	inline EditorCamera& GetCamera() { return mCamera; }
	inline std::vector<Mesh>& MergedMeshes() { return m_CachedMeshes; }
	inline SceneGPUStorage* GPUStorage() { return m_GPUStorage.get(); }
};


KRS_END_NAMESPACE