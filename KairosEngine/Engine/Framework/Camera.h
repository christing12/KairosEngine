#pragma once


#include <DirectXTK12/Inc/SimpleMath.h>

namespace Kairos {
	using namespace DirectX::SimpleMath;

	class Camera {
	public:

		Camera() = default;
		Camera(const Matrix& projectionMatrix, Vector3 startPos = Vector3::Zero);
		virtual ~Camera() = default;

		const Vector3& GetPos() const { return mPosition; }
		const Matrix& GetViewMat() const { return mViewMat; }
		Matrix GetViewProjMat() const { return mViewMat * mProjectionMat; }
		const Matrix& GetProjMat() const { return mProjectionMat; }
		void SetProjMat(const Matrix& mat) { mProjectionMat = mat; }
		void SetRotation(const Vector3& vec) { mRotation = vec; }


		Quaternion GetOrientation() const;
		Vector3 GetUpDir() const;
		Vector3 GetRightDir() const;
		Vector3 GetForward() const;

	protected:
		Vector3 CalculatePosition();
		void UpdateCameraView();


	protected:
		Matrix mProjectionMat = Matrix::Identity;
		Matrix mViewMat = Matrix::Identity;
		Vector3 mPosition = Vector3::Zero;
		Vector3 mRotation = Vector3(0.0f, 3.14159, 0.0f);

		Vector3 mTarget = Vector3::Zero;
		float mDistFromTarget = 0.0f;

	};
}