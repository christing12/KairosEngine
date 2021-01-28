#include "krspch.h"
#include "Scene.h"
#include "RenderBackend/RHI/RenderDevice.h"
#include "RenderBackend/RHI/Texture.h"
#include "RenderBackend/RHI/CommandContext.h"
#include "SceneGPUStorage.h"

using namespace DirectX;

namespace Kairos
{

	void Scene::Init(RenderDevice* pDevice)
	{
		m_Device = pDevice;
		mCamera = EditorCamera(XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f), Vector3(0, 0, -75));
		m_GPUStorage = CreateScope<SceneGPUStorage>(m_Device, this);
	}

	void Scene::Update(float deltaTime)
	{
		mCamera.Update(0.01f);
	}

	MeshComponent& Scene::AddMeshInstance(MeshComponent&& meshInstance)
	{
		m_MeshInstances.emplace_back(std::move(meshInstance));
		return m_MeshInstances.back();
	}

	Mesh& Scene::AddMesh(Mesh&& mesh)
	{
		m_CachedMeshes.emplace_back(std::move(mesh));
		return m_CachedMeshes.back();
	}

	Material& Scene::AddMaterial(Material&& mat)
	{
		m_Materials.emplace_back(std::move(mat));
		return m_Materials.back();
	}

	Light& Scene::AddLight(Light&& light)
	{
		m_Lights.emplace_back(std::move(light));
		return m_Lights.back();
	}

	bool Scene::OnEvent(Event& e)
	{
		mCamera.OnEvent(e);
		return true;
	}

	Material* Scene::LoadMaterial(const std::string& name)
	{
		return nullptr;
	}

}
