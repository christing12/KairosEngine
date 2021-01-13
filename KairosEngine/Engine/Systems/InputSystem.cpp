#include "krspch.h"
#include "InputSystem.h"

#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/WindowEvent.h"

#include "Interface/IEngine.h"

extern Kairos::IEngine* g_Engine;

namespace Kairos {

	namespace InputSystemInternal {
		bool keyState[256] = { false };

		bool leftMouseButton = false;
		bool rightMouseButton = false;
		bool middleMouseButton = false;

		float mouseX = 0, mouseY = 0;
	}

	using namespace InputSystemInternal;


	bool InputSystem::Setup(ISystemConfig* config)
	{
		auto systemConfig = reinterpret_cast<IInputConfig*>(config);


		return true;
	}

	bool InputSystem::Init()
	{
		return true;
	}

	bool InputSystem::Update()
	{
		return true;
	}

	bool InputSystem::Shutdown()
	{
		return true;
	}



	bool InputSystem::IsKeyDown(KeyCode keycode)
	{
		return keyState[keycode];
	}

	bool InputSystem::IsMouseButtonDown(MouseCode mousecode)
	{
		switch (mousecode)
		{
		case Mouse::LeftButton: return leftMouseButton; break;
		case Mouse::RightButton: return rightMouseButton; break;
		case Mouse::MiddleButton: return middleMouseButton; break;

		}
		return false;
	}

	std::pair<float, float> InputSystem::GetMousePosition()
	{
		POINT point;
		GetCursorPos(&point);
		return  { (float)point.x, (float)point.y };
	}


	void InputSystem::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(InputSystem::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(InputSystem::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(InputSystem::OnKeyReleased));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(InputSystem::OnMousePressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(InputSystem::OnMouseReleased));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(InputSystem::OnMouseMoved));
	}

	bool InputSystem::OnWindowClose(WindowCloseEvent& e) {
		return true;
	}

	bool InputSystem::OnKeyPressed(KeyPressedEvent& e) {
		keyState[e.GetKeyCode()] = true;
		return true;
	}
	bool InputSystem::OnKeyReleased(KeyReleasedEvent& e)
	{
		//std::cout << e.ToString() << std::endl;
		keyState[e.GetKeyCode()] = false;
		return true;
	}
	bool InputSystem::OnMousePressed(MouseButtonPressedEvent& e)
	{

		switch (e.GetMouseButton())
		{
		case Mouse::LeftButton: leftMouseButton = true; break;
		case Mouse::RightButton: rightMouseButton = true; break;
		case Mouse::MiddleButton: middleMouseButton = true; break;
		}
		//std::cout << e.ToString() << std::endl;
		return true;
	}
	bool InputSystem::OnMouseReleased(MouseButtonReleasedEvent& e)
	{
		switch (e.GetMouseButton())
		{
		case Mouse::LeftButton: leftMouseButton = false; break;
		case Mouse::RightButton: rightMouseButton = false; break;
		case Mouse::MiddleButton: middleMouseButton = false; break;
		}
		return true;
	}

	bool InputSystem::OnMouseMoved(MouseMovedEvent& e)
	{
		mouseX = e.GetX();
		mouseY = e.GetY();
		return true;
	}


}

