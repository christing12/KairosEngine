#pragma once

#include <Core/EngineCore.h>

KRS_BEGIN_NAMESPACE(Kairos)


class ApplicationEntry {
public:
	ApplicationEntry() = default;
	virtual ~ApplicationEntry() = default;

	virtual bool Setup(void* appHook, void* extrahook, char* cmdLine);
	virtual bool Initialize();
	virtual bool Run();

	virtual bool OnEvent(class Event& e);
	virtual void Update() {}
	virtual void Render() {}
	virtual bool Shutdown();

protected:
	Scope<class Engine> m_Engine;

	virtual bool OnKeyPressed(class KeyPressedEvent& e);
	virtual bool OnWindowResize(class WindowResizeEvent& e);



};

KRS_END_NAMESPACE