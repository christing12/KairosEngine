#include <API.h>

struct CameraData {
	Matrix viewProjMat;
	Vector3 cameraPos;
};

struct FrameBuffer {
	std::shared_ptr<Kairos::Texture> ColorBuffer;
	std::shared_ptr<Kairos::Texture> DepthBuffer;
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
	Kairos::EditorCamera mCamera;
public:
	void SetupResources();

private:
	FrameBuffer mFrameBuffers[2];

private:
	Matrix cameraProjMat; // this will store our projection matrix
	Matrix cameraViewMat; // this will store our view matrix

	Vector3 cameraPosition; // this is our cameras position vector
	Vector3 cameraTarget; // a vector describing the point in space our camera is looking at
	Vector3 cameraUp; // the worlds up vector

	Matrix cube1WorldMat; // our first cubes world matrix (transformation matrix)
	Matrix cube1RotMat; // this will keep track of our rotation for the first cube
	Vector3 cube1Position; // our first cubes position in space
};