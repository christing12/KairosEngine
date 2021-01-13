#pragma once
#include "ISystem.h"

#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

#include "Core/Events/Event.h"


KRS_BEGIN_NAMESPACE(Kairos)

struct IInputConfig : public ISystemConfig {

};

class IInputSystem : public ISystem {
public:
	KRS_INTERFACE_NON_COPYABLE(IInputSystem); 

	//virtual void OnEvent(class Event& e);

	virtual bool IsKeyDown(KeyCode keycode) = 0;
	virtual bool IsMouseButtonDown(MouseCode mousecode) = 0;
	virtual std::pair<float, float> GetMousePosition() = 0;
};


KRS_END_NAMESPACE