#pragma once


#include <DirectXTK12/Inc/SimpleMath.h>

namespace Kairos {
	using namespace DirectX::SimpleMath;

	class Camera {
	public:

		Camera() = default;
		Camera(const Matrix& projectionMatrix);
		virtual ~Camera() = default;

		const Matrix& GetProjMat() const { return m_ProjectionMat; }
		void SetProjMat(const Matrix& mat) { m_ProjectionMat = mat; }

	protected:
		Matrix m_ProjectionMat = Matrix::Identity;
	};
}