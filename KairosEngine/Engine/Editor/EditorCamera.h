#pragma once
#include "Scene/Camera.h"
#include <Core/Events/MouseEvent.h>

namespace Kairos {
	class EditorCamera : public Camera {
	public:
		EditorCamera() = default;
		EditorCamera(const dxmath::Matrix& projMat, dxmath::Vector3 startPos = dxmath::Vector3::Zero);

		void Update(float deltaTime);
		void OnEvent(Event& e);

	private:
		bool OnMouseScroll(MouseScrolledEvent& event);

		void MousePan(const dxmath::Vector2& delta);
		void MouseRotate(const dxmath::Vector2& delta);
		void MouseZoom(float delta);

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		dxmath::Vector2 mInitialMousePos;
		Uint32 width = 1024.f, height = 1024.0f;

	};
}