#include <API.h>

#pragma once

using namespace DirectX;
struct ConstantBuffer {
	Matrix modelMat;
	Matrix viewProjMat;
};

struct SkyboxCB {
	Matrix viewRotProjMat;
};

struct Light {
	Vector3 diffuseColor = Vector3(1.0f, 0.0f, 0.0f);
	float padding;
	Vector3 specularColor = Vector3(0.0f, 1.0f, 1.0f);
	float padding2;
	Vector3 position = Vector3::Forward;
	float specularPower = 0.5f;
};

struct CameraData {
	Matrix viewProjMat;
	Vector3 cameraPos;
};

class MeshRenderer : public Kairos::Application {
public:
	MeshRenderer() = default;

	void DoSomething();
	virtual void OnEvent(Kairos::Event& e) override final;

	virtual void Update(float deltaTime) override final;
	virtual void Shutdown() override final;
	virtual void InitEngine() override final;
	virtual void Render() override final;
	virtual void Present() override final;

	std::string projectDir;

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
	SkyboxCB cbSkybox;
	CameraData m_CameraData;
private:
	std::shared_ptr<Kairos::PipelineStateObject> m_WhitePSO;
	std::shared_ptr<Kairos::RootSignature> m_WhiteRS;
	std::shared_ptr<Kairos::Shader> m_WhiteVS;
	std::shared_ptr<Kairos::Shader> m_WhitePS;


private:
	std::shared_ptr<Kairos::PipelineStateObject> mSkyboxPSO;
	std::shared_ptr<Kairos::RootSignature> mSkyboxSignature;
	std::shared_ptr<Kairos::Mesh> mSkyboxMesh;
	std::shared_ptr<Kairos::Buffer> skyboxConstantBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE skyboxCPUHandle;
	D3D12_INDEX_BUFFER_VIEW skyboxIView;
	D3D12_VERTEX_BUFFER_VIEW skyboxVView;

	std::shared_ptr<Kairos::Texture> skyboxTexture;
private:
	Kairos::EngineIMGUI mEditor;
	Kairos::EditorCamera mCamera;


private:
	void LoadPBRTextures();
	std::shared_ptr<Kairos::Texture> mMetalTex;
	std::shared_ptr<Kairos::Texture> mNormalMap;
	std::shared_ptr<Kairos::Texture> mRoughnessMap;


private:
	Light light;

	Matrix cameraProjMat; // this will store our projection matrix
	Matrix cameraViewMat; // this will store our view matrix

	Vector3 cameraPosition; // this is our cameras position vector
	Vector3 cameraTarget; // a vector describing the point in space our camera is looking at
	Vector3 cameraUp; // the worlds up vector

	Matrix cube1WorldMat; // our first cubes world matrix (transformation matrix)
	Matrix cube1RotMat; // this will keep track of our rotation for the first cube
	Vector3 cube1Position; // our first cubes position in space
};
