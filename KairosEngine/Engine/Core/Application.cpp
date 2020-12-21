#include "krspch.h"
#include "Application.h"
#include "MouseCodes.h"
#include "Graphics/Core/Renderer.h"

#include "Window.h"
#include "Input.h"

namespace Kairos {
	void Application::Create()
	{
		Log::Init();
		mWindow = CreateRef<Window>(WindowProps());
		mWindow->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		Input::Init();
		mRenderer = Renderer(mWindow->GetNativeWindow());
	}

	void Application::Run()
	{
		std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
		float deltaTime = 1.0f / 60.0f;
		while (true) {
			const auto ecode = mWindow->ProcessEvents();
			if (ecode) {
				KRS_CORE_WARN("ENDING GAME");
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

	void Application::OnEvent(Event& e)
	{
		Input::sInstance->OnEvent(e);
	}

	void Application::Update(float deltaTime)
	{
	}

	void Application::Shutdown()
	{
	}
}