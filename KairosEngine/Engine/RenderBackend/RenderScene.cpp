#include "krspch.h"
#include "RenderScene.h"

#include "RenderDevice.h"
#include "GraphicsTypes.h"
#include "CommandContext.h"
#include "Mesh.h"
#include "BufferManager.h"

namespace Kairos {
	void RenderScene::Init(RenderDevice* pDevice)
	{
		m_Device = pDevice;
		

	}
	void RenderScene::MergeMeshes(BufferManager* manager)
	{
		KRS_CORE_ASSERT(m_Device != nullptr, "Render Scene has not been initialized");

		size_t totalVerts = 0;
		size_t totalIndices = 0;

		for (auto& mesh : m_Meshes)
		{
			mesh.FirstIndex = static_cast<Uint32>(totalIndices);
			mesh.FirstVertex = static_cast<Uint32>(totalVerts);

			totalVerts += mesh.NumVertices;
			totalIndices += mesh.NumIndices;

			mesh.isMerged = true;
		}

		
		m_MergedVertices = m_Device->CreateVertBuffer(static_cast<Uint32>(totalVerts), sizeof(Vertex));
		m_MergedIndices = m_Device->CreateIndexBuffer(static_cast<Uint32>(totalIndices), sizeof(Uint32));



		m_Device->TransientCommand([&](CommandContext& ctx)
		{
			ctx.TransitionResource(m_MergedVertices, D3D12_RESOURCE_STATE_COPY_DEST);
			ctx.TransitionResource(m_MergedIndices, D3D12_RESOURCE_STATE_COPY_DEST, true);
			for (auto& mesh : m_Meshes)
			{

				ctx.CopyBufferRegion(m_MergedVertices, mesh.FirstVertex * sizeof(Vert), (void*)(mesh.original->Vertices.data()), mesh.NumVertices * sizeof(Vert));

				ctx.CopyBufferRegion(m_MergedIndices, mesh.FirstVertex * sizeof(Uint32), (void*)(mesh.original->Indices.data()), mesh.NumIndices * sizeof(Uint32));

			}
			ctx.TransitionResource(m_MergedVertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			ctx.TransitionResource(m_MergedVertices, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		});

	}
}