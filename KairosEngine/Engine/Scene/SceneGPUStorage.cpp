#include "krspch.h"
#include "Scene.h"
#include "SceneGPUStorage.h"


#include "Material.h"
#include "RenderBackend/RHI/Texture.h"
#include "RenderBackend/RHI/RenderDevice.h"
#include "RenderBackend/RHI/CommandContext.h"
#include "RenderBackend/RHI/Buffer.h"

namespace Kairos {
	SceneGPUStorage::SceneGPUStorage(RenderDevice* pDevice, Scene* scene)
		: m_Device(pDevice)
		, m_Scene(scene)
	{}

	void SceneGPUStorage::UploadMaterials()
	{
		KRS_CORE_ASSERT(m_Scene != nullptr, "Scene has been assigned yet to upload materials");

		Uint32 entrySize = sizeof(GPUMaterialEntry);
		Uint32 size = entrySize * m_Scene->m_Materials.size();
		m_MatTableBuffer = m_Device->CreateStaticBuffer(size, size, nullptr);

		Uint32 matIdx = 0;

		//TODO: Copy Queue?
		CommandContext& ctx = m_Device->AllocateCommandContext(CommandType::Graphics);
		ctx.TransitionResource(*m_MatTableBuffer, ResourceState::CopyDest, true);
		for (Material& mat : m_Scene->m_Materials)
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

	void SceneGPUStorage::UploadMeshes()
	{
		Uint32 totalVerts = 0;
		Uint32 totalIndices = 0;
		for (Mesh& mesh : m_Scene->m_CachedMeshes)
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
			for (Mesh& mesh : m_Scene->m_CachedMeshes)
			{
				ctx.CopyBufferRegion(*m_MergedVertexBuffer, sizeof(Vertex) * mesh.TempLocation.firstVert, static_cast<void*>(mesh.Vertices().data()), sizeof(Vertex) * mesh.Vertices().size());
				ctx.CopyBufferRegion(*m_MergedIndexBuffer, sizeof(Uint32) * mesh.TempLocation.firstIndex, static_cast<void*>(mesh.Indices().data()), sizeof(Uint32) * mesh.Indices().size());
			}
			ctx.TransitionResource(*m_MergedVertexBuffer, ResourceState::ConstantBuffer);
			ctx.TransitionResource(*m_MergedIndexBuffer, ResourceState::Common);
			});
	}

	void SceneGPUStorage::UploadLights()
	{
		Uint32 totalSize = m_Scene->m_Lights.size() * sizeof(Light);
		m_LightTableBuffer = m_Device->CreateStaticBuffer(totalSize, sizeof(Light), nullptr);

		CommandContext& ctx = m_Device->AllocateCommandContext(CommandType::Graphics);
		ctx.TransitionResource(*m_LightTableBuffer, ResourceState::CopyDest, true);
		int dest = 0;
		for (Light& light : m_Scene->m_Lights)
		{
			ctx.CopyBufferRegion(*m_LightTableBuffer, sizeof(Light) * dest, &light, sizeof(Light));
			++dest;
		}
		ctx.TransitionResource(*m_LightTableBuffer, ResourceState::Common);
		ctx.Submit(true);
	}


}

