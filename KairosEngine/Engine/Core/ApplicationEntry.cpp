#include "krspch.h"
#include "ApplicationEntry.h"

#include "Systems/Engine.h"

#include "Systems/WinWindowSystem.h"
#include "Systems/InputSystem.h"

#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/WindowEvent.h"

using namespace Kairos;

bool ApplicationEntry::Setup(void* appHook, void* extrahook, char* cmdLine)
{
	m_Engine = CreateScope<Engine>();
	if (!m_Engine.get()) {
		KRS_CORE_ERROR("ERRO INTIALIZING ENWIF");
		return false;
	}

	if (!m_Engine->Setup(appHook, extrahook, cmdLine)) {
		KRS_CORE_ERROR("ERRRRRROR");
		return false;
	}
	return true;
}

bool ApplicationEntry::Initialize()
{
	if (!m_Engine->Init()) {
		KRS_CORE_ERROR("Error withs setting up");
		return false;
	}
	m_Engine->GetWindowSystem()->SetEventCallback(BIND_EVENT_FN(ApplicationEntry::OnEvent));

	return true;
}

bool ApplicationEntry::Run()
{
	std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
	float deltaTime = 1.0f / 60.0f;
	while (1) {
		if (!m_Engine->GetWindowSystem()->Update()) {
			KRS_CORE_INFO("Ending Application");
			break;
		}

		else {
			std::chrono::high_resolution_clock::time_point frameEnd;
			do
			{
				frameEnd = std::chrono::high_resolution_clock::now();
				double duration = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(frameEnd - frameStart).count();
				deltaTime = (float)(0.000000001 * duration);
			} while (deltaTime < 0.9f / 60.0f);     // giving it a 10% buffer
			frameStart = frameEnd;
		}
		Update();
		m_Engine->Update();
		Render();
	}

	return m_Engine->Run();
}

bool ApplicationEntry::OnEvent(Event& e)
{
	reinterpret_cast<InputSystem*>(m_Engine->GetInputSystem())->OnEvent(e);
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ApplicationEntry::OnKeyPressed));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(ApplicationEntry::OnWindowResize));

	return false;
}

bool ApplicationEntry::Shutdown()
{
	return m_Engine->Shutdown();
}

bool ApplicationEntry::OnKeyPressed(KeyPressedEvent& e)
{
	return true;
}

bool ApplicationEntry::OnWindowResize(WindowResizeEvent& e)
{
	return true;
}
