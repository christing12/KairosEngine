#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "Shader.h"

#include "GraphicsTypes.h"
#include "ShaderCompiler.h"

#include <filesystem>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

class ShaderManager {
public:
	ShaderManager() = default;
	ShaderManager(RenderDevice* pDevice, const std::filesystem::path& workingDirectory);
	~ShaderManager() {}

	Shader* LoadShader(ShaderType type, const std::string& entryPoint, const std::string& relativePath);


private:
	Scope<class ShaderCompiler> m_Compiler;

	RenderDevice* m_Device;
	std::unordered_map<std::string, Shader> m_ShaderCache;

	std::filesystem::path m_ExecutablePath;
	std::filesystem::path m_ShaderSrcPath;
};


KRS_END_NAMESPACE