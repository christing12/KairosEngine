#pragma once

#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"

#include "Framework/Camera.h"

KRS_BEGIN_NAMESPACE(Kairos)

class Scene {
public:
	Scene() = default;

	void Serialize(std::string filePath);
	void Deserialize(std::string filePath, Uint32 width = 1024, Uint32 height = 1024);

private:
	Camera mCamera;

};


KRS_END_NAMESPACE