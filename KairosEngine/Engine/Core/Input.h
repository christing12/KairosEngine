#pragma once

#include "EngineCore.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"

#include <map>

KRS_BEGIN_NAMESPACE(Kairos)

class Input {
	friend class Application;
public:
	static void Init();
	void OnEvent(Event& e);
public:
	static bool IsKeyDown(KeyCode keycode);
	static bool IsMouseButtonDown(MouseCode mousecode);
	static std::pair<float, float> Input::GetMousePosition();

private:
	Input() = default;

	bool OnWindowClose(WindowCloseEvent& e);
	bool OnKeyPressed(KeyPressedEvent& e);
	bool OnKeyReleased(KeyReleasedEvent& e);
	bool OnMousePressed(MouseButtonPressedEvent& e);
	bool OnMouseReleased(MouseButtonReleasedEvent& e);
private:
	static Input* sInstance;
	bool keyState[256] = { false };
};

KRS_END_NAMESPACE