#pragma once
#include "Graphics/Core/Camera.h"
#include <Core/Events/MouseEvent.h>

namespace Kairos {
	class EditorCamera : public Camera {
	public:
		EditorCamera() = default;
		EditorCamera(const Matrix& projMat);

		void Update(float deltaTime);

		void OnEvent(Event& e);


		const Vector3& GetPos() const { return m_Position; }
		const Matrix& GetViewMat() const { return m_ViewMatrix; }
		Matrix GetViewProjMat() const { return m_ViewMatrix * m_ProjectionMat; }

		Quaternion GetOrientation() const;
		Vector3 GetUpDir() const;
		Vector3 GetRightDir() const;
		Vector3 GetForward() const;

	private:
		void UpdateCameraView();
		bool OnMouseScroll(MouseScrolledEvent& event);

		void MousePan(const Vector2& delta);
		void MouseRotate(const Vector2& delta);
		void MouseZoom(float delta);

		

		Vector3 CalculatePosition();

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		Vector2 mInitialMousePos;

		Vector3 m_Position, m_Rotation, m_Target;
		Matrix m_ViewMatrix;

		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_DistFromTarget = 0.0f;
		Uint32 width = 1280.0f, height = 1024.0f;
	};
}