#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

KRS_BEGIN_NAMESPACE(Kairos)

struct Light {
	DirectX::SimpleMath::Vector4 radiance;
	DirectX::SimpleMath::Vector3 direction;
};




KRS_END_NAMESPACE