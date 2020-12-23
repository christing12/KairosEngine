#include "krspch.h"
#include "Application.h"
#include "MouseCodes.h"
#include "Graphics/Core/Renderer.h"
#include "Graphics/Core/RenderDevice.h"

#include "Input.h"

namespace Kairos {
	void Application::Create()
	{
		mWindow.Init(1280, 1024);
		mWindow.SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		Input::Init();
		mRenderer = CreateScope<Renderer>();
		mRenderer->Initialize(mWindow);
	}

	void Application::Run()
	{
		std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
		float deltaTime = 1.0f / 60.0f;
		while (mIsRunning) {
			const auto ecode = mWindow.ProcessEvents();
			if (ecode) {
				KRS_CORE_WARN("ENDING GAME");
				mIsRunning = false;
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
			Update(deltaTime);
			Render();
			Present();
		}
	}

	void Application::InitEngine() {
	}



	void Application::OnEvent(Event& e)
	{
		Input::sInstance->OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Application::OnKeyPressed));
	}

	void Application::Update(float deltaTime)
	{
	}

	void Application::Shutdown()
	{
		mRenderer->m_Device->Flush();
	}


	bool Application::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::Escape) {
			mIsRunning = false;
		}
		return false;
	}
}