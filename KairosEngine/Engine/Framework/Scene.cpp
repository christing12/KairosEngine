#include "krspch.h"
#include "Scene.h"
#include "Util/JSONUtil.h"

#include <DirectXMath.h>
#include "nlohmann/json.hpp"
#include <string>
#include <fstream>

namespace Kairos {
	void Scene::Serialize(std::string filePath)
	{
	}

	void Scene::Deserialize(std::string filePath, Uint32 width, Uint32 height)
	{
		using json = nlohmann::json;

		std::ifstream ifs(filePath.c_str());
		KRS_CORE_ASSERT(!ifs.fail(), "Issue with loading scene file");

		json scene = json::parse(ifs);

		json metadata = scene["metadata"];

		std::string filepath = metadata["filepath"];

		//
		{
			json cameraData = scene["camera"];
			Vector3 translation = Util::Vector3FromJSON(cameraData["position"]);
			float fov = cameraData["fov"];
			float nearPlane = cameraData["nearPlane"];
			float farPlane = cameraData["farPlane"];

			mCamera = Camera(DirectX::XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), width / height, 0.1f, 1000.0f), translation);
		}
	}

}

