#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>

#include "Core/Events/Event.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/WindowEvent.h"


KRS_BEGIN_NAMESPACE(Kairos)


class InputSystem {
public:
	InputSystem() = default;
	~InputSystem() = default;

	bool Setup(struct ISystemConfig* config);
	bool Init();
	bool Update();
	bool Shutdown();

	void OnEvent(class Event& e);


	bool IsKeyDown(KeyCode keycode);
	bool IsMouseButtonDown(MouseCode mousecode);
	std::pair<float, float> GetMousePosition();

private:
	bool OnWindowClose(class WindowCloseEvent& e);
	bool OnKeyPressed(class KeyPressedEvent& e);
	bool OnKeyReleased(class KeyReleasedEvent& e);
	bool OnMousePressed(class MouseButtonPressedEvent& e);
	bool OnMouseReleased(class MouseButtonReleasedEvent& e);
	bool OnMouseMoved(class MouseMovedEvent& e);
};

KRS_END_NAMESPACE