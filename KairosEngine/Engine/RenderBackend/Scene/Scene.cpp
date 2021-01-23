#include "krspch.h"
#include "Scene.h"
#include "RenderBackend/RHI/RenderDevice.h"
#include "RenderBackend/RHI/Texture.h"
#include "RenderBackend/RHI/CommandContext.h"

using namespace DirectX;

namespace Kairos
{

	void Scene::Init(RenderDevice* pDevice)
	{
		m_Device = pDevice;
		mCamera = EditorCamera(XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f), Vector3(0, 0, -5));
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

	void Scene::Bind(CommandContext& context)
	{
		GraphicsContext& gfx = context.GetGraphicsContext();
		gfx.BindBuffer(*m_MergedVertexBuffer, 0, 0, ShaderRegister::ShaderResource);
		gfx.BindBuffer(*m_MergedIndexBuffer, 1, 0, ShaderRegister::ShaderResource);
		gfx.BindBuffer(*m_MatTableBuffer, 2, 0, ShaderRegister::ShaderResource);
		gfx.BindBuffer(*m_LightTableBuffer, 3, 0, ShaderRegister::ShaderResource);


		//gfx.DrawInstanced(m_CachedMeshes.back().Indices().size(), 1, 0, 0);

		
	}

	void Scene::UploadMaterials()
	{
		Uint32 entrySize = sizeof(GPUMaterialEntry);
		Uint32 size = entrySize * m_Materials.size();
		m_MatTableBuffer = m_Device->CreateStaticBuffer(size, size, nullptr);

		Uint32 matIdx = 0;

		//TODO: Copy Queue?
		CommandContext& ctx = m_Device->AllocateCommandContext(CommandType::Graphics);
		ctx.TransitionResource(*m_MatTableBuffer, ResourceState::CopyDest, true);
		for (Material& mat : m_Materials)
		{
			GPUMaterialEntry entry{
				mat.Albedo->GetSRDescriptor().Index(),
				mat.NormalMap->GetSRDescriptor().Index(),
				mat.Metalness->GetSRDescriptor().Index(),
				mat.Roughness->GetSRDescriptor().Index()
			};
			ctx.CopyBufferRegion(*m_MatTableBuffer, entrySize * matIdx, &entry, entrySize);
			++matIdx;
		}
		ctx.TransitionResource(*m_MatTableBuffer, ResourceState::Common);
		ctx.Submit(true);


	}

	void Scene::UploadMeshes()
	{
		Uint32 totalVerts = 0;
		Uint32 totalIndices = 0;
		for (Mesh& mesh : m_CachedMeshes)
		{
			mesh.TempLocation = Mesh::BufferLocation{ totalVerts, totalIndices };

			totalVerts += mesh.Vertices().size();
			totalIndices += mesh.Indices().size();

			
		}

		m_MergedVertexBuffer = m_Device->CreateStaticBuffer(totalVerts * sizeof(Vertex), sizeof(Vertex), nullptr);
		m_MergedIndexBuffer = m_Device->CreateStaticBuffer(totalIndices * sizeof(Uint32), sizeof(Uint32), nullptr);

		m_Device->TransientCommand([&](CommandContext& ctx) {
			ctx.TransitionResource(*m_MergedVertexBuffer, ResourceState::CopyDest, true);
			ctx.TransitionResource(*m_MergedIndexBuffer, ResourceState::CopyDest, true);
			for (Mesh& mesh : m_CachedMeshes)
			{
				ctx.CopyBufferRegion(*m_MergedVertexBuffer, sizeof(Vertex) * mesh.TempLocation.firstVert, static_cast<void*>(mesh.Vertices().data()), sizeof(Vertex) * mesh.Vertices().size());
				ctx.CopyBufferRegion(*m_MergedIndexBuffer, sizeof(Uint32) * mesh.TempLocation.firstIndex, static_cast<void*>(mesh.Indices().data()), sizeof(Uint32) * mesh.Indices().size());
			}
			ctx.TransitionResource(*m_MergedVertexBuffer, ResourceState::ConstantBuffer);
			ctx.TransitionResource(*m_MergedIndexBuffer, ResourceState::Common);
		});

		m_ITest.BufferLocation = m_MergedVertexBuffer->GPUVirtualAdress();
		m_ITest.Format = DXGI_FORMAT_R32_UINT;
		m_ITest.SizeInBytes = m_MergedIndexBuffer->Size();

		m_VTest.BufferLocation = m_MergedVertexBuffer->GPUVirtualAdress();
		m_VTest.SizeInBytes = m_MergedVertexBuffer->Size();
		m_VTest.StrideInBytes = m_MergedVertexBuffer->Stride();
	}


	void Scene::UploadLights()
	{
		Uint32 totalSize = m_Lights.size() * sizeof(Light);
		m_LightTableBuffer = m_Device->CreateStaticBuffer(totalSize, sizeof(Light), nullptr);

		CommandContext& ctx = m_Device->AllocateCommandContext(CommandType::Graphics);
		ctx.TransitionResource(*m_LightTableBuffer, ResourceState::CopyDest, true);
		int dest = 0;
		for (Light& light : m_Lights)
		{
			ctx.CopyBufferRegion(*m_LightTableBuffer, sizeof(Light) * dest, &light, sizeof(Light));
			++dest;
		}
		ctx.TransitionResource(*m_LightTableBuffer, ResourceState::Common);
		ctx.Submit(true);
	}

	Material* Scene::LoadMaterial(const std::string& name)
	{
		return nullptr;
	}

}
