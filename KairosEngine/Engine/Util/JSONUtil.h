#pragma once

#include <DirectXTK/SimpleMath.h>
#include "nlohmann_json/json.hpp"

namespace Util {
	DirectX::SimpleMath::Vector3 Vector3FromJSON(nlohmann::json jsonObj) {
		return Vector3(float(jsonObj[0]), float(jsonObj[1]), float(jsonObj[2]));
	}
}