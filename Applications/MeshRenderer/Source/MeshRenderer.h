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
	
private:
	void TestCubes(Kairos::GraphicsContext& context);
	XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
	XMFLOAT4X4 cameraViewMat; // this will store our view matrix

	XMFLOAT4 cameraPosition; // this is our cameras position vector
	XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
	XMFLOAT4 cameraUp; // the worlds up vector

	XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
	XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
	XMFLOAT4 cube1Position; // our first cubes position in space

	XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
	XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
	XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

	int numCubeIndices; // the number of indices to draw the cube

};
