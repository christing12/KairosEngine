#include <API.h>

#pragma once

using namespace DirectX;
struct ConstantBuffer {
	DirectX::XMFLOAT4X4 mvpMat;
	Vector4 padding[48];
};


class MeshRenderer : public Kairos::Application {
public:
	MeshRenderer() = default;

	void DoSomething();

	virtual void InitEngine() override final;
	virtual void Render() override final;
	virtual void Present() override final;

private:
	std::shared_ptr<Kairos::Buffer> mVertexBuffer;
	std::shared_ptr<Kairos::Buffer> mIndexBuffer;
	std::shared_ptr<Kairos::PipelineStateObject> mPSO;
	std::shared_ptr<Kairos::RootSignature> mSig;
	std::shared_ptr<Kairos::Texture> mTexture;

	D3D12_INDEX_BUFFER_VIEW mIBView;
	D3D12_VERTEX_BUFFER_VIEW mVBView;

	std::shared_ptr<Kairos::Shader> mVertexShader;
	std::shared_ptr<Kairos::Shader> mPixelShader;

	std::shared_ptr<Kairos::Mesh> mMesh;

	ConstantBuffer cbPerObject;

private:
	void TestCubes(Kairos::GraphicsContext& context);
	Matrix cameraProjMat; // this will store our projection matrix
	Matrix cameraViewMat; // this will store our view matrix

	Vector3 cameraPosition; // this is our cameras position vector
	Vector3 cameraTarget; // a vector describing the point in space our camera is looking at
	Vector3 cameraUp; // the worlds up vector

	Matrix cube1WorldMat; // our first cubes world matrix (transformation matrix)
	Matrix cube1RotMat; // this will keep track of our rotation for the first cube
	Vector3 cube1Position; // our first cubes position in space
};
