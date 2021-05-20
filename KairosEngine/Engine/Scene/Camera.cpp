#include "krspch.h"
#include "Camera.h"

namespace Kairos {
	Camera::Camera(const Matrix& projectionMatrix, Vector3 startPos)
		: mProjectionMat(projectionMatrix)
        , mPosition(startPos)
	{
        mDistFromTarget = Vector3::Distance(mPosition, mTarget);
        UpdateCameraView();
	}


    Vector3 Camera::CalculatePosition()
    {
        return mTarget - GetForward() * mDistFromTarget;
    }


    void Camera::UpdateCameraView()
    {
        mPosition = CalculatePosition();

        Quaternion orientation = GetOrientation();
        mViewRotMat = Matrix::CreateFromQuaternion(orientation);

        mViewMat = mViewRotMat * Matrix::CreateTranslation(mPosition);
        mViewMat = mViewMat.Invert();

       // mViewMat = DirectX::XMMatrixLookAtLH(mPosition, Vector3::Zero, Vector3::Up);
    }


    Quaternion Camera::GetOrientation() const
    {
        // y x z
        return Quaternion::CreateFromYawPitchRoll(mRotation.y, mRotation.x, 0.0f);

        // return Quaternion(Vector3(-m_Pitch, -m_Yaw, 0.0f));
    }

    Vector3 Camera::GetUpDir() const
    {
        return Vector3::Transform(Vector3::Up, GetOrientation());
    }

    Vector3 Camera::GetRightDir() const
    {
        return Vector3::Transform(Vector3::Right, GetOrientation());
    }

    Vector3 Camera::GetForward() const
    {
        return Vector3::Transform(Vector3::Backward, GetOrientation());
    }

    GPUCamera Camera::GetGPUStruct()
    {
        GPUCamera gpu;
        gpu.Position = mPosition;
        gpu.View = mViewMat;
        gpu.ViewProjection = mViewMat * mProjectionMat;
        gpu.Projection = mProjectionMat;
        gpu.InverseProjection = mProjectionMat.Invert();
        gpu.InverseView = mViewMat.Invert();
        gpu.InverseViewProjection = gpu.ViewProjection.Invert();
        

        return gpu;
    }

}