#pragma once

#include <Graphics/Core/GraphicsTypes.h>

struct aiMesh;

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class Buffer;

//
//struct MeshBuffer {
//	Buffer VertexBuffer;
//	Buffer IndexBuffer;
//	D3D12_VERTEX_BUFFER_VIEW VBufferView;
//	D3D12_INDEX_BUFFER_VIEW IBufferView;
//
//	MeshBuffer()
//};
//typedef struct MeshBuffer MeshBuffer;


class Mesh {
public:
	D3D12_VERTEX_BUFFER_VIEW GetVertexView() const;
	D3D12_INDEX_BUFFER_VIEW GetIndexView() const;

	static std::shared_ptr<Mesh> CreateFromFile(RenderDevice* pDevice, const char* file);
	Mesh(class RenderDevice* pDevice, const aiMesh* mesh);

	std::vector<Vertex> mVertices;
	std::vector<Uint16> mIndices;
private:


	class RenderDevice* m_Device;
	Ref<Buffer> m_VertexBuffer;
	Ref<Buffer> m_IndexBuffer;


};

KRS_END_NAMESPACE
