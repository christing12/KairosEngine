#pragma once

#include "Core/EngineCore.h"
#include <string>

KRS_BEGIN_NAMESPACE(Kairos)

enum class EventType
{
	None = 0,
	WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
	KeyPressed, KeyReleased, KeyTyped,
	MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

/*
	Use of bitfields because events can belong to more than one category
	Can |= them together ex: 0011 would be application & input
*/
enum EventCategory
{
	None = 0,
	EventCategoryApplication = BIT(0),
	EventCategoryInput = BIT(1),
	EventCategoryKeyboard = BIT(2),
	EventCategoryMouse = BIT(3),
	EventCategoryMouseButton = BIT(4)
};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategories() const override { return category; }


// want this to be an abstract class
class Event {
public:
	friend class EventDispatcher;
	virtual EventType GetEventType() const = 0;
	virtual const char* GetName() const = 0;
	virtual int GetCategories() const = 0;
	virtual std::string ToString() const { return GetName(); }

	inline bool InCategory(EventCategory category) {
		return GetCategories() & category;
	}

	bool Handled = false;
};

class EventDispatcher {
public:
	EventDispatcher(Event& event) : m_Event(event) {

	}

	// no type safety right now (its fine tho?)
	template<typename T, typename F>
	bool Dispatch(const F& func) {
		if (m_Event.GetEventType() == T::GetStaticType())
		{
			m_Event.Handled = func(static_cast<T&>(m_Event));
			return true;
		}
		return false;
	}
private:
	// i guess we don't want a pointer because Events only last for so long? don't want null ptr
	Event& m_Event;
};

inline std::ostream& operator << (std::ostream& os, const Event& e) {
	return os << e.ToString();
}


KRS_END_NAMESPACE