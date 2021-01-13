#include <API.h>

#pragma once

using namespace DirectX;


struct SceneSettings
{

	static const int NumLights = 3;
	struct Light {
		Vector3 direction;
		Vector3 radiance;
	} lights[NumLights];
};


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
	Vector3 specularColor = Vector3(1.f, 1.f, 1.f);
	float padding2;
	Vector3 position = 2 * Vector3::Right;
	float specularPower = 32;
};

struct CameraData {
	Matrix viewProjMat;
	Vector3 cameraPos;
};

struct FrameBuffer {
	std::shared_ptr<Kairos::Texture> ColorBuffer;
	std::shared_ptr<Kairos::Texture> DepthBuffer;
};

class MeshRenderer : public Kairos::Application {
public:
	SceneSettings g_SceneSettings;
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
	FrameBuffer mFrameBuffers[2];

	Kairos::Ref<Kairos::RootSignature> mTonemapSig;
	Kairos::Ref<Kairos::GraphicsPSO> mTonemapPSO;
	Kairos::Ref<Kairos::Shader> TonemapVS;
	Kairos::Ref<Kairos::Shader> TonemapPS;

private:
	std::shared_ptr<Kairos::Buffer> mVertexBuffer;
	std::shared_ptr<Kairos::Buffer> mIndexBuffer;
	std::shared_ptr<Kairos::GraphicsPSO> mPSO;
	std::shared_ptr<Kairos::RootSignature> mSig;
	std::shared_ptr<Kairos::Texture> mTexture;

	std::shared_ptr<Kairos::Shader> mVertexShader;
	std::shared_ptr<Kairos::Shader> mPixelShader;

	std::shared_ptr<Kairos::Mesh> mMesh;
	std::shared_ptr<Kairos::Mesh> mMainMesh;

	ConstantBuffer cbPerObject;
	SkyboxCB cbSkybox;
	CameraData m_CameraData;
private:
	std::shared_ptr<Kairos::GraphicsPSO> m_WhitePSO;
	std::shared_ptr<Kairos::RootSignature> m_WhiteRS;
	std::shared_ptr<Kairos::Shader> m_WhiteVS;
	std::shared_ptr<Kairos::Shader> m_WhitePS;


private:
	std::shared_ptr<Kairos::GraphicsPSO> mSkyboxPSO;
	std::shared_ptr<Kairos::RootSignature> mSkyboxSignature;
	std::shared_ptr<Kairos::Mesh> mSkyboxMesh;
	std::shared_ptr<Kairos::Buffer> skyboxConstantBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE skyboxCPUHandle;

	std::shared_ptr<Kairos::Texture> skyboxTexture;
	std::shared_ptr<Kairos::Texture> envTextureUnfilter;
	std::shared_ptr<Kairos::Texture> m_irmapTexture;
	std::shared_ptr<Kairos::Texture> m_envTexture;
	std::shared_ptr<Kairos::Texture> m_spBRDF_LUT;
private:
	std::shared_ptr<Kairos::ComputePSO> m_computePSO1;
	std::shared_ptr<Kairos::ComputePSO> m_computePSO2;
	std::shared_ptr<Kairos::ComputePSO> m_computePSO3;
	std::shared_ptr<Kairos::ComputePSO> m_computePSO4;
	std::shared_ptr<Kairos::ComputePSO> m_computePSO5;


private:
	Kairos::EngineIMGUI mEditor;
	Kairos::EditorCamera mCamera;


private:
	void LoadPBRTextures();
	void CreateMeshes();
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
