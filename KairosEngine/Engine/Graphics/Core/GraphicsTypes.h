#pragma once

#include <DirectXTK12/Inc/SimpleMath.h>

using namespace DirectX::SimpleMath;

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector3 texCoord;
};

struct VertTex {
	VertTex() {}
	VertTex(float px, float py, float pz, float u, float v)
		: pos(px, py, pz)
		, texCoord(u, v)
	{}
	VertTex(Vector3 inPos, Vector2 inTexCoord) : pos(inPos), texCoord(inTexCoord) {}
	Vector3 pos;
	Vector2 texCoord;
};

enum BUFFER_TYPE : uint8_t {
	BUFFER_VERTEX = 0,
	BUFFER_INDEX,
	BUFFER_CONSTANT
};

enum LINEAR_ALLOCATOR_TYPE : uint8_t {
	GPU_EXCLUSIVE = 1,
	CPU_WRITABLE = GPU_EXCLUSIVE << 1
};