#pragma once


#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"
#include "GraphicsTypes.h"
#include "ShaderCompiler.h"
#include <filesystem>


namespace Kairos {
	class RenderDevice;

	struct Shader
	{
		Shader() = default;
		Shader(const Microsoft::WRL::ComPtr<IDxcBlob>& code, const std::string& entryPoint, ShaderType type)
			: Code(code)
			, EntryPoint(entryPoint)
			, Type(type)
		{}

		ShaderType Type;
		Microsoft::WRL::ComPtr<IDxcBlob> Code;
		std::string EntryPoint;
	};
	

	class ShaderManager {
	public:
		KRS_CLASS_NON_COPYABLE_AND_MOVABLE(ShaderManager);
		ShaderManager(RenderDevice* pDevice, const std::filesystem::path& workingDirectory);
		
		Shader* LoadShader(ShaderType type, const std::string& entryPoint, const std::string& relativePath);
		
	
	private:
		ShaderCompiler m_Compiler;

		RenderDevice* m_Device;
		std::unordered_map<std::string, Shader> m_ShaderCache;

		std::filesystem::path m_ExecutablePath;
		std::filesystem::path m_ShaderSrcPath;
	};
}