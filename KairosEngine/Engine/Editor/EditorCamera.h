#pragma once
#include "Framework/Camera.h"
#include <Core/Events/MouseEvent.h>

namespace Kairos {
	class EditorCamera : public Camera {
	public:
		EditorCamera() = default;
		EditorCamera(const Matrix& projMat, Vector3 startPos = Vector3::Zero);

		void Update(float deltaTime);
		void OnEvent(Event& e);

	private:
		bool OnMouseScroll(MouseScrolledEvent& event);

		void MousePan(const Vector2& delta);
		void MouseRotate(const Vector2& delta);
		void MouseZoom(float delta);

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		Vector2 mInitialMousePos;
		Uint32 width = 1280.0f, height = 1024.0f;
	};
}