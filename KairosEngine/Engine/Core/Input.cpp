#include "krspch.h"
#include "Input.h"

namespace Kairos {
	void Input::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Input::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Input::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(Input::OnKeyReleased));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(Input::OnMousePressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(Input::OnMouseReleased));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(Input::OnMouseMoved));
	}

	bool Input::IsKeyDown(KeyCode keycode)
	{
		return Input::Get()->keyState[keycode];
	}

	bool Input::IsMouseButtonDown(MouseCode mousecode)
	{
		switch (mousecode)
		{
		case Mouse::LeftButton: return Input::Get()->leftMouseButton; break;
		case Mouse::RightButton: return Input::Get()->rightMouseButton; break;
		case Mouse::MiddleButton: return Input::Get()->middleMouseButton; break;

		}
		return false;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		POINT point;
		GetCursorPos(&point);
		return  { (float)point.x, (float)point.y };
	}

	bool Input::OnWindowClose(WindowCloseEvent& e) {
		return true;
	}

	bool Input::OnKeyPressed(KeyPressedEvent& e) {
		std::cout << e.ToString() << std::endl;
		Input::Get()->keyState[e.GetKeyCode()] = true;
		return true;
	}
	bool Input::OnKeyReleased(KeyReleasedEvent& e)
	{
		//std::cout << e.ToString() << std::endl;
		Input::Get()->keyState[e.GetKeyCode()] = false;
		return true;
	}
	bool Input::OnMousePressed(MouseButtonPressedEvent& e)
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
	bool Input::OnMouseReleased(MouseButtonReleasedEvent& e)
	{
		switch (e.GetMouseButton())
		{
		case Mouse::LeftButton: leftMouseButton = false; break;
		case Mouse::RightButton: rightMouseButton = false; break;
		case Mouse::MiddleButton: middleMouseButton = false; break;
		}
		return true;
	}

	bool Input::OnMouseMoved(MouseMovedEvent& e)
	{
		mouseX = e.GetX();
		mouseY = e.GetY();
		return true;
	}

}