#include "krspch.h"
#include "ShaderManager.h"
#include "ShaderCompiler.h"

#include "GraphicsTypes.h"

namespace Kairos {
	ShaderManager::ShaderManager(RenderDevice* pDevice, const std::filesystem::path& workingDirectory)
		: m_Device(pDevice)
		, m_ExecutablePath(workingDirectory)
	{
		m_Compiler = CreateScope<ShaderCompiler>();
		m_ShaderSrcPath = m_ExecutablePath / "Data/shaders";
	}


	Shader* ShaderManager::LoadShader(ShaderType type, const std::string& entryPoint, const std::string& relativePath)
	{
		// TODO: Check For Null Results (more info in SHader Compile Result)
		ShaderCompileResult result = m_Compiler->CompileShader(m_ShaderSrcPath / relativePath, entryPoint, type);


		Shader shader;
		shader.Code = result.Code;
		shader.EntryPoint = entryPoint;
		shader.Type = type;
		m_ShaderCache.insert(std::pair<std::string, Shader>(relativePath + entryPoint, std::move(shader)));


		return &m_ShaderCache[relativePath + entryPoint];
	}
}