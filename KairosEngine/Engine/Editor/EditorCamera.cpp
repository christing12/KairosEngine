#include "krspch.h"
#include "EditorCamera.h"
#include "Core/Input.h"

namespace Kairos {
    EditorCamera::EditorCamera(const Matrix& projMat)
        : Camera(projMat)
    {
        m_Rotation = Vector3(90.0f, 0.0f, 0.0f);
        m_Target = Vector3(0.0f, 0.0f, 0.0f);

        m_DistFromTarget = Vector3::Distance(Vector3(-5.f, 5.f, 5.f), m_Target);
        m_Yaw = 3.0f * Math::Pi / 4.0f;
        m_Pitch = Math::Pi / 4.0f;

        UpdateCameraView();
    }
    void EditorCamera::Update(float deltaTime)
    {
        if (Input::IsKeyDown(Key::Q)) {
            auto [x, y] = Input::GetMousePosition();
            Vector2 mouse(x, y);
            Vector2 delta = Vector2(mouse - mInitialMousePos) * 0.003f;
            mInitialMousePos = mouse;

            if (Input::IsMouseButtonDown(Mouse::MiddleButton))
                MousePan(delta);
            else if (Input::IsMouseButtonDown(Mouse::LeftButton))
                MouseRotate(delta);
            else if (Input::IsMouseButtonDown(Mouse::RightButton))
                MouseZoom(delta.y);
        }
        UpdateCameraView();
    }
    void EditorCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::OnMouseScroll));
    }

    Quaternion EditorCamera::GetOrientation() const
    {
        return Quaternion(Vector3(-m_Pitch, -m_Yaw, 0.0f));
    }

    Vector3 EditorCamera::GetUpDir() const
    {
        return Vector3::Transform(Vector3::Up, GetOrientation());
    }

    Vector3 EditorCamera::GetRightDir() const
    {
        return Vector3::Transform(Vector3::Right, GetOrientation());
    }

    Vector3 EditorCamera::GetForward() const
    {
        return Vector3::Transform(Vector3::Forward, GetOrientation());
    }

    void EditorCamera::UpdateCameraView()
    {
        m_Position = CalculatePosition();

        Quaternion orientation = GetOrientation();
        m_ViewMatrix = Matrix::CreateTranslation(m_Position) * Matrix::CreateFromQuaternion(orientation);
        m_ViewMatrix = m_ViewMatrix.Invert();

    }
    bool EditorCamera::OnMouseScroll(MouseScrolledEvent& event)
    {
        float delta = event.GetYOffset() * 0.1f;
        MouseZoom(delta);
        UpdateCameraView();
        return false;
    }

    void EditorCamera::MousePan(const Vector2& delta)
    {
        auto [xSpeed, ySpeed] = PanSpeed();
        m_Target += GetRightDir() * delta.x * xSpeed * m_DistFromTarget;
        m_Target += GetUpDir() * delta.y * ySpeed * m_DistFromTarget;
    }

    void EditorCamera::MouseRotate(const Vector2& delta)
    {
        float yawSign = GetUpDir().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yawSign * delta.x * RotationSpeed();
        m_Pitch += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float delta)
    {
        m_DistFromTarget -= delta * ZoomSpeed();
        if (m_DistFromTarget < 1.0f) {
            m_Target += GetForward();
            m_DistFromTarget = 1.0f;
        }
    }

    Vector3 EditorCamera::CalculatePosition()
    {
        return m_Target - GetForward() * m_DistFromTarget;
    }


    std::pair<float, float> EditorCamera::PanSpeed() const
    {
        float x = Math::Min<float>(width / 1000.0f, 2.4f);
        float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = Math::Min<float>(height / 1000.0f, 2.4f); // max = 2.4f
        float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return { xFactor, yFactor };
    }

    float EditorCamera::RotationSpeed() const
    {
        return 0.0f;
    }
    float EditorCamera::ZoomSpeed() const
    {
        float distance = m_DistFromTarget * 0.2f;
        distance = Math::Max<float>(distance, 0.0f);
        float speed = distance * distance;

        speed = Math::Min<float>(speed, 100.0f);
        return speed;

    }
}