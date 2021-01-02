#pragma once

#include <Core/EngineCore.h>
#include <Core/NonCopyable.h>

KRS_BEGIN_NAMESPACE(Kairos)

class AssetManager : NonCopyable {
public:
	static AssetManager* Instance() {
		static AssetManager instance;
		return &instance;
	}

	void Setup();


private:
	std::string m_RootDirectory;
	std::string m_ContentPath;
	std::string m_ConfigPath;
	std::string m_ShaderPath;

};


KRS_END_NAMESPACE