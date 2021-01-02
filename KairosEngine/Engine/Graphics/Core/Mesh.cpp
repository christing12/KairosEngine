#include "krspch.h"
#include "Mesh.h"
#include "RenderDevice.h"
#include "Buffer.h"

#include "GraphicsTypes.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Kairos {

	D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexView() const
	{
		return m_VertexBuffer->VertexBufferView(0, m_VertexBuffer->GetBufferSize(), sizeof(Vertex));
	}
	D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexView() const
	{
		return m_IndexBuffer->IndexBufferView(0, m_IndexBuffer->GetBufferSize(), false);
	}

	Ref<Mesh> Mesh::CreateFromFile(RenderDevice* pDevice, const char* file)
	{

		Ref<Mesh> krsMesh = nullptr;
		Assimp::Importer imp;
		const aiScene* model = imp.ReadFile(file,
			aiProcess_MakeLeftHanded |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices
		);

		if (model->HasMeshes()) {
			krsMesh = CreateRef<Mesh>(pDevice, model->mMeshes[0]);
		}
		return krsMesh;
	}
	
	Mesh::Mesh(RenderDevice* pDevice, const aiMesh* mesh)
	{
		mVertices.reserve(mesh->mNumVertices);
		for (size_t i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			vertex.position = *reinterpret_cast<Vector3*>(&mesh->mVertices[i]);
			vertex.normal = *reinterpret_cast<Vector3*>(&mesh->mNormals[i]);
			if (mesh->HasTangentsAndBitangents()) {
				vertex.tangent = *reinterpret_cast<Vector3*>(&mesh->mTangents[i]);
				vertex.bitangent = *reinterpret_cast<Vector3*>(&mesh->mBitangents[i]);
			}
			if (mesh->HasTextureCoords(0))
				vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			mVertices.push_back(vertex);
		}

		mIndices.reserve(mesh->mNumFaces * 3);
		for (size_t i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];
			//	KRS_CORE_ASSERT(face.mIndices == 3, "Faces not traingles");
			mIndices.push_back(face.mIndices[0]);
			mIndices.push_back(face.mIndices[1]);
			mIndices.push_back(face.mIndices[2]);
		}

		pDevice->CreateBuffer(m_VertexBuffer, mVertices.size(), sizeof(Vertex), &mVertices[0], L"Mesh Vert Buffer");
		pDevice->CreateBuffer(m_IndexBuffer, mIndices.size(), sizeof(Uint16), &mIndices[0], L"Mesh Index Buffer");
	}
}