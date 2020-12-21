#include "krspch.h"
#include "Mesh.h"
#include "RenderDevice.h"
#include "Buffer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Kairos {
	Mesh::Mesh(RenderDevice* pDevice, const char* filename)
		: m_Device(pDevice)
	{
		Vector2 test(0.f, 0.f);
		Assimp::Importer imp;
		const aiScene* model = imp.ReadFile(filename,
			aiProcess_MakeLeftHanded |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices
		);

		const aiMesh* mesh = model->mMeshes[0];

		std::vector<VertTex> vertices;
		vertices.reserve(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; i++) {
			vertices.push_back({
				*reinterpret_cast<Vector3*>(&mesh->mVertices[i]),
				*reinterpret_cast<Vector2*>(&mesh->mNormals[i])
				});
		}

		m_Vertices = vertices;

		std::vector<Uint16> indices;
		indices.reserve(mesh->mNumFaces * 3);
		for (size_t i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
		//	KRS_CORE_ASSERT(face.mIndices == 3, "Faces not traingles");
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		m_Indices = indices;

		pDevice->CreateBuffer(m_VertexBuffer, vertices.size(), sizeof(VertTex), &vertices[0], L"Mesh Vert Buffer");
		pDevice->CreateBuffer(m_IndexBuffer, indices.size(), sizeof(Uint16), &indices[0], L"Mesh Index Buffer");
	}

	D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexView() const
	{
		return m_VertexBuffer->VertexBufferView(0, m_VertexBuffer->GetBufferSize(), sizeof(VertTex));
	}
	D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexView() const
	{
		return m_IndexBuffer->IndexBufferView(0, m_IndexBuffer->GetBufferSize(), false);
	}
}