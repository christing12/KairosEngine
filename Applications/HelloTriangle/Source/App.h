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


struct PerFrameRootConstants
{
	Kairos::GPUCamera camera;

	//GPUCamera CurrentFrameCamera;
	//GPUCamera PreviousFrameCamera;
	//// Cameras are 16 byte aligned
	//uint32_t IsDenoiserEnabled;
	//uint32_t IsReprojectionHistoryDebugEnabled;
	//uint32_t IsGradientDebugEnabled;
	//uint32_t IsMotionDebugEnabled;
	//uint32_t IsDenoiserAntilagEnabled;
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
	
	Kairos::Scene m_Scene;
	GlobalRootConstants mGlobalConstants;
	PerFrameRootConstants mPerFrameConstants; 

	D3D12_RECT m_Scissor;
	D3D12_VIEWPORT m_Viewport;

	Kairos::Descriptor linearSamplerHandle;
	Kairos::Descriptor aniSamplerHandler;
private:
	FrameBuffer mFrameBuffers[2];
	Kairos::RenderHandle bufferHandle;
	Kairos::DynamicBuffer* m_SkyboxBuffer;

	Kairos::Mesh skyboxMesh;

private:
	GBufferPass m_GBufferPass;
	TonemapPass m_TonemapPass;
	SkyboxPass m_SkyboxPass;
	Kairos::EnvironmentMap m_EnvMap;
};