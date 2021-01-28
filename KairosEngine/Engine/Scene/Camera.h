#pragma once


#include <DirectXTK/SimpleMath.h>

namespace dxmath = DirectX::SimpleMath;

namespace Kairos {
	
	struct GPUCamera {
		DirectX::SimpleMath::Vector3 Position;
		float padding;
		// 16 byte boundary
		DirectX::SimpleMath::Matrix View;
		DirectX::SimpleMath::Matrix Projection;
		DirectX::SimpleMath::Matrix ViewProjection;
		DirectX::SimpleMath::Matrix InverseView;
		DirectX::SimpleMath::Matrix InverseProjection;
		DirectX::SimpleMath::Matrix InverseViewProjection;
	};

	class Camera {
	public:

		Camera() = default;
		Camera(const DirectX::SimpleMath::Matrix& projectionMatrix, DirectX::SimpleMath::Vector3 startPos = DirectX::SimpleMath::Vector3::Zero);
		virtual ~Camera() = default;

		const DirectX::SimpleMath::Vector3& GetPos() const { return mPosition; }
		const DirectX::SimpleMath::Matrix& GetViewMat() const { return mViewMat; }
		DirectX::SimpleMath::Matrix GetViewProjMat() const { return mViewMat * mProjectionMat; }
		const DirectX::SimpleMath::Matrix& GetProjMat() const { return mProjectionMat; }
		const DirectX::SimpleMath::Matrix& GetViewRotMat() const { return mViewRotMat; }
		void SetProjMat(const DirectX::SimpleMath::Matrix& mat) { mProjectionMat = mat; }
		void SetRotation(const DirectX::SimpleMath::Vector3& vec) { mRotation = vec; }


		dxmath::Quaternion GetOrientation() const;
		DirectX::SimpleMath::Vector3 GetUpDir() const;
		DirectX::SimpleMath::Vector3 GetRightDir() const;
		DirectX::SimpleMath::Vector3 GetForward() const;

		GPUCamera GetGPUStruct();

	protected:
		void UpdateCameraView();


	protected:
		DirectX::SimpleMath::Vector3 CalculatePosition();
		DirectX::SimpleMath::Matrix mProjectionMat = dxmath::Matrix::Identity;
		DirectX::SimpleMath::Matrix mViewMat = dxmath::Matrix::Identity;
		DirectX::SimpleMath::Matrix mViewRotMat = dxmath::Matrix::Identity;
		DirectX::SimpleMath::Vector3 mPosition = dxmath::Vector3::Zero;
		dxmath::Vector3 mRotation = dxmath::Vector3(0.0f, 3.14159, 0.0f);

		dxmath::Vector3 mTarget = dxmath::Vector3::Zero;
		float mDistFromTarget = 0.0f;

	};
}