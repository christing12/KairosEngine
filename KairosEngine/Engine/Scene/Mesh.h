#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

namespace dxmath = DirectX::SimpleMath;


KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
struct Vertex {
	dxmath::Vector3 position;
	dxmath::Vector3 normal;
	dxmath::Vector3 tangent;
	dxmath::Vector3 bitangent;
	dxmath::Vector2 texCoord;
	float padding;
};

class Mesh {
public:
	Mesh() = default;
	~Mesh() = default;
	Mesh(std::vector<Vertex> vertices, std::vector<Uint32> indices) : m_Vertices(vertices), m_Indices(indices) {}

	inline std::vector<Vertex>& Vertices() { return m_Vertices; }
	inline std::vector<Uint32>& Indices() { return m_Indices; }


	static Mesh LoadFromAsset(RenderDevice* pDevice, const std::string& filename);


	struct BufferLocation {
		Uint32 firstVert = 0;
		Uint32 firstIndex = 0;

	};
	BufferLocation TempLocation;
private:
	std::vector<Vertex> m_Vertices;
	// TODO: For now its just Uint32 indices, possibly option for Uint16?
	std::vector<Uint32> m_Indices;
};



KRS_END_NAMESPACE