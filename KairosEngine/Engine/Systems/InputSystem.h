#pragma once

#include "Interface/IInputSystem.h"

#include "Core/Events/Event.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/WindowEvent.h"


KRS_BEGIN_NAMESPACE(Kairos)


class InputSystem : public IInputSystem {
public:

	virtual bool Setup(ISystemConfig* config) override final;
	virtual bool Init() override final;
	virtual bool Update() override final;
	virtual bool Shutdown() override final;

	void OnEvent(class Event& e);


	virtual bool IsKeyDown(KeyCode keycode) override;
	virtual bool IsMouseButtonDown(MouseCode mousecode) override;
	virtual std::pair<float, float> GetMousePosition() override;

private:
	bool OnWindowClose(class WindowCloseEvent& e);
	bool OnKeyPressed(class KeyPressedEvent& e);
	bool OnKeyReleased(class KeyReleasedEvent& e);
	bool OnMousePressed(class MouseButtonPressedEvent& e);
	bool OnMouseReleased(class MouseButtonReleasedEvent& e);
	bool OnMouseMoved(class MouseMovedEvent& e);
};

KRS_END_NAMESPACE