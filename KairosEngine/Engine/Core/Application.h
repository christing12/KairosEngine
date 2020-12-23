#pragma once

#include "EngineCore.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"
#include "Window.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct AppCreateInfo {

};

class Application {
public:
	Application() = default;
	virtual ~Application() = default;

	void Create();
	void Run();
	virtual void OnEvent(Event& e);

public:
	virtual void Update(float deltaTime);
	virtual void Shutdown();
	virtual void InitEngine();

	virtual void Render() {}
	virtual void Present() {}

	bool OnKeyPressed(KeyPressedEvent& e);
protected:
	bool mIsRunning = true;
	Window mWindow;
	Scope<class Renderer> mRenderer;
};

extern Application* CreateApplication();

KRS_END_NAMESPACE