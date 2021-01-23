#include <API.h>
#include <DirectXTK/SimpleMath.h>



using namespace DirectX::SimpleMath;

struct CameraData {
	Matrix viewProjMat;
	Vector3 cameraPos;
};

struct FrameBuffer {
	Kairos::Texture* ColorBuffer;
	Kairos::Texture* DepthBuffer;
};



struct GlobalRootConstants
{
	Uint32 AnisotropicClampSamplerIdx;
	Uint32 LinearClampSamplerIdx;
	Uint32 PointClampSamplerIdx;
	Uint32 MinSamplerIdx;
	Uint32 MaxSamplerIdx;
};

struct GPUCamera
{
	Vector3 Position;
	// 16 byte boundary
	Matrix View;
	Matrix Projection;
	Matrix ViewProjection;
	Matrix InverseView;
	Matrix InverseProjection;
	Matrix InverseViewProjection;
	// 16 byte boundary
	float NearPlane = 0.0f;
	float FarPlane = 0.0f;
	float ExposureValue100 = 0.0f;
	float FoVH = 0.0f;
	// 16 byte boundary
	float FoVV = 0.0f;
	float FoVHTan = 0.0f;
	float FoVVTan = 0.0f;
	float AspectRatio = 0;
	// 16 byte boundary
};

struct PerFrameRootConstants
{
	Matrix ViewProjection;
	Vector3 Position;

	//GPUCamera CurrentFrameCamera;
	//GPUCamera PreviousFrameCamera;
	//// Cameras are 16 byte aligned
	//uint32_t IsDenoiserEnabled;
	//uint32_t IsReprojectionHistoryDebugEnabled;
	//uint32_t IsGradientDebugEnabled;
	//uint32_t IsMotionDebugEnabled;
	//uint32_t IsDenoiserAntilagEnabled;
};

struct SkyboxPass
{
	Matrix skyboxViewProj;
	Uint32 indexBufferOffset;
	Uint32 vertexBufferOffset;
	Uint32 skyboxTextureIndex;
};

using namespace Kairos;
class App : public Kairos::ApplicationEntry {
public:
	App() = default;

	virtual bool Initialize() override final;
	virtual void Update() override final;
	virtual void Render() override final;
	virtual bool Shutdown() override final;

	virtual bool OnEvent(class Kairos::Event& e) override final;

	std::string projectDir;
public:
	void SetupResources();

private:
	Scene m_Scene;
	GlobalRootConstants mGlobalConstants;
	PerFrameRootConstants mPerFrameConstants; 

	D3D12_RECT m_Scissor;
	D3D12_VIEWPORT m_Viewport;

	Kairos::Descriptor linearSamplerHandle;
private:
	FrameBuffer mFrameBuffers[2];
	Kairos::RenderHandle bufferHandle;
	Kairos::Texture* envTexture;
	Kairos::Texture* unfilteredEnv;
	Kairos::DynamicBuffer* m_SkyboxBuffer;

	Kairos::Mesh skyboxMesh;
};