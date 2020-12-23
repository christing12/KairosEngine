#include "krspch.h"
#include "Camera.h"

namespace Kairos {
	Camera::Camera(const Matrix& projectionMatrix)
		: m_ProjectionMat(projectionMatrix)
	{
	}
}