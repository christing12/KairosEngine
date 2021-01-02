#include "krspch.h"
#include "AssetManager.h"

#include "Filesystem.h"

namespace Kairos {
	void AssetManager::Setup() {
		m_RootDirectory = Filesystem::GetExecutablePath();

		m_ContentPath = m_RootDirectory + "\\Content\\";
		if (!Filesystem::PathExists(m_ContentPath))
			KRS_CORE_ERROR("Content Path: " + m_ConfigPath + " doesn't exist ");

		m_ConfigPath = m_RootDirectory + "\\Config\\";
		if (!Filesystem::PathExists(m_ConfigPath))
			KRS_CORE_ERROR("Config Path: " + m_ConfigPath + " doesn't exist\n");

		m_ShaderPath = m_RootDirectory + "\\Shaders\\";
		if (!Filesystem::PathExists(m_ShaderPath))
			KRS_CORE_ERROR("Shder Path: " + m_ShaderPath + " doesn't exist\n");

	}
}