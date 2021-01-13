#include "krspch.h"
//#include "Application.h"
//#include "MouseCodes.h"
//#include "RenderBackend/Renderer.h"
//#include "RenderBackend/RenderDevice.h"
//#include "Filesystem.h"
//#include "Input.h"
//
//namespace Kairos {
//	void Application::Create()
//	{
//		mWindow.Init(1024, 1024);
//		mWindow.SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
//		mInput = Input::Get();
//		mRenderer = CreateScope<Renderer>();
//		mRenderer->Initialize(mWindow);
//	}
//
//	void Application::Run()
//	{
//		std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();
//		float deltaTime = 1.0f / 60.0f;
//		while (mIsRunning) {
//			const auto ecode = mWindow.ProcessEvents();
//			if (ecode) {
//				KRS_CORE_WARN("ENDING GAME");
//				mIsRunning = false;
//				break;
//			}
//			else {
//				std::chrono::high_resolution_clock::time_point frameEnd;
//				do
//				{
//					frameEnd = std::chrono::high_resolution_clock::now();
//					double duration = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(frameEnd - frameStart).count();
//					deltaTime = (float)(0.000000001 * duration);
//				} while (deltaTime < 0.9f / 60.0f);     // giving it a 10% buffer
//				frameStart = frameEnd;
//			}
//			Update(deltaTime);
//			Render();
//			Present();
//		}
//	}
//
//	void Application::InitEngine() {
//	}
//
//
//
//	void Application::OnEvent(Event& e)
//	{
//		mInput->OnEvent(e);
//
//		EventDispatcher dispatcher(e);
//		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Application::OnKeyPressed));
//		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));
//	}
//
//	void Application::Update(float deltaTime)
//	{
//	}
//
//	void Application::Shutdown()
//	{
//		mRenderer->m_Device->Flush();
//	}
//
//
//	bool Application::OnKeyPressed(KeyPressedEvent& e)
//	{
//		switch (e.GetKeyCode())
//		{
//		case Key::Escape: mIsRunning = false; break;
//		case Key::P: {
//			//KRS_CORE_INFO(Filesystem::GetExecutablePath());
//			//KRS_CORE_INFO(Filesystem::GetWorkingDirectory());
//			KRS_CORE_INFO(Filesystem::GetRelativePath("C:/Users/Chris Ting/Desktop/Link/"));
//			//Filesystem::OpenDirectoryWindow("C:/Users/Chris Ting/Desktop");
//		}
//		}
//		return false;
//	}
//	bool Application::OnWindowResize(WindowResizeEvent& e)
//	{
//		//mRenderer->GetRenderDevice()->Flush();
//		
//		return false;
//	}
//}