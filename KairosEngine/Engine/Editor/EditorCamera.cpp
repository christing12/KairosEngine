#include "krspch.h"
#include "EditorCamera.h"
#include "Systems/Engine.h"
#include "Systems/InputSystem.h"


extern Kairos::Engine* g_Engine;

namespace Kairos {
    EditorCamera::EditorCamera(const Matrix& projMat, Vector3 startPos)
        : Camera(projMat, startPos)
    {
        mRotation = Vector3(0.0f, 0.0f, 0.0f);
        mTarget = Vector3(0.0f, 0.0f, 0.0f);

    //    mDistFromTarget = Vector3::Distance(m_Position, m_Target);

     //   UpdateCameraView();
    }
    void EditorCamera::Update(float deltaTime)
    {
        if (g_Engine->GetInputSystem()->IsKeyDown(Key::LeftAlt)) {
            auto [x, y] = g_Engine->GetInputSystem()->GetMousePosition();
            Vector2 mouse(x, y);
            Vector2 delta = Vector2(mouse - mInitialMousePos) * 0.003f;
            mInitialMousePos = mouse;

            if (g_Engine->GetInputSystem()->IsMouseButtonDown(Mouse::MiddleButton))
                MousePan(delta);
            else if (g_Engine->GetInputSystem()->IsMouseButtonDown(Mouse::LeftButton))
                MouseRotate(delta);
            else if (g_Engine->GetInputSystem()->IsMouseButtonDown(Mouse::RightButton))
                MouseZoom(delta.y);
        }
        UpdateCameraView();
    }
    void EditorCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::OnMouseScroll));
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
        mTarget += GetRightDir() * delta.x * -xSpeed * mDistFromTarget;
        mTarget += GetUpDir() * delta.y * ySpeed * mDistFromTarget;
    }

    void EditorCamera::MouseRotate(const Vector2& delta)
    {
        float yawSign = GetUpDir().y < 0 ? -1.0f : 1.0f;
        mRotation.y += yawSign * delta.x * RotationSpeed();
        mRotation.x += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float delta)
    {
        mDistFromTarget -= delta * ZoomSpeed();
        if (mDistFromTarget < 1.0f) {
            mTarget += GetForward();
            mDistFromTarget = 1.0f;
        }
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
        return 0.9f;
        //return 0.0f;
    }
    float EditorCamera::ZoomSpeed() const
    {
        float distance = mDistFromTarget * 0.2f;
        distance = Math::Max<float>(distance, 0.0f);
        float speed = distance * distance;

        speed = Math::Min<float>(speed, 100.0f);
        return speed;

    }
}