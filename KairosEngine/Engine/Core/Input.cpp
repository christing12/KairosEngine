#include "krspch.h"
#include "Input.h"

namespace Kairos {

	Input* Input::sInstance = nullptr;

	void Input::Init()
	{
		KRS_CORE_ASSERT(sInstance == nullptr, "Input has already been initialized");
		sInstance = &Input();
		
	}

	void Input::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Input::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Input::OnKeyPressed));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(Input::OnKeyReleased));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(Input::OnMousePressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(Input::OnMouseReleased));
	}

	bool Input::IsKeyDown(KeyCode keycode)
	{
		return false;
	}

	bool Input::IsMouseButtonDown(MouseCode mousecode)
	{
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
		return true;
	}
	bool Input::OnKeyReleased(KeyReleasedEvent& e)
	{
		std::cout << e.ToString() << std::endl;
		return true;
	}
	bool Input::OnMousePressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::LeftButton)
		{
			auto [x, y] = Input::GetMousePosition();
			std::cout << x << " " << y << std::endl;
		}
		//std::cout << e.ToString() << std::endl;
		return true;
	}
	bool Input::OnMouseReleased(MouseButtonReleasedEvent& e)
	{
		std::cout << e.ToString() << std::endl;
		return true;
	}

}