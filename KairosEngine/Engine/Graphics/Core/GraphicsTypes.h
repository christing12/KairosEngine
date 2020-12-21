#pragma once

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

struct Vertex {
	Vertex() {}
	Vertex(float px, float py, float pz, float c1, float c2, float c3, float c4)
		: pos(px, py, pz)
		, color(c1, c2, c3, c4)
	{}
	Vertex(Vector3 inPos, Vector4 inCol) : pos(inPos), color(inCol) {}

	Vector3 pos;
	Vector4 color;
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