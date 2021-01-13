#include <API.h>

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



class App : public Kairos::ApplicationEntry {
public:
	App() = default;

	virtual bool Initialize() override final;
	virtual void Update() override final;
	virtual void Render() override final;
	virtual bool Shutdown() override final;

	virtual bool OnEvent(class Kairos::Event& e) override final;

	std::string projectDir;
	SceneSettings g_SceneSettings;

public:
	void SetupResources();
	void LoadPBRTextures();
	void CreateMeshes();

private:
	FrameBuffer mFrameBuffers[2];
	ConstantBuffer cbPerObject;
	SkyboxCB cbSkybox;
	CameraData m_CameraData;
	Kairos::EditorCamera mCamera;


	Kairos::Ref<Kairos::Shader> TonemapVS;
	Kairos::Ref<Kairos::Shader> TonemapPS;

	std::shared_ptr<Kairos::Shader> mVertexShader;
	std::shared_ptr<Kairos::Shader> mPixelShader;
private:
	std::shared_ptr<Kairos::Buffer> mVertexBuffer;
	std::shared_ptr<Kairos::Buffer> mIndexBuffer;
	std::shared_ptr<Kairos::Buffer> skyboxConstantBuffer;


	std::shared_ptr<Kairos::Mesh> mMesh;
	std::shared_ptr<Kairos::Mesh> mMainMesh;
	std::shared_ptr<Kairos::Mesh> mSkyboxMesh;


	std::shared_ptr<Kairos::Texture> skyboxTexture;
	std::shared_ptr<Kairos::Texture> envTextureUnfilter;
	std::shared_ptr<Kairos::Texture> m_irmapTexture;
	std::shared_ptr<Kairos::Texture> m_envTexture;
	std::shared_ptr<Kairos::Texture> m_spBRDF_LUT;

	std::shared_ptr<Kairos::Texture> mTexture;
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