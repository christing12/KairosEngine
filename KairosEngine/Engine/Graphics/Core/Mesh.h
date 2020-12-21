#pragma once

KRS_BEGIN_NAMESPACE(Kairos)

class Mesh {
public:
	Mesh(class RenderDevice* pDevice, const char* filename);

	D3D12_VERTEX_BUFFER_VIEW GetVertexView() const;
	D3D12_INDEX_BUFFER_VIEW GetIndexView() const;



	std::vector<VertTex> m_Vertices;
	std::vector<Uint16> m_Indices;
private:
	class RenderDevice* m_Device;
	Ref<class Buffer> m_VertexBuffer;
	Ref<class Buffer> m_IndexBuffer;
};

KRS_END_NAMESPACE
