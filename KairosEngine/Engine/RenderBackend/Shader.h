#pragma once


#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"
#include "GraphicsTypes.h"
#include <filesystem>


namespace Kairos {
	struct ShaderCreateInfo {
		ShaderType Type;
		std::wstring FilePath;
		const Char* EntryPoint;

		const void* ByteCode;
		size_t ByteCodeSize;

		ShaderCreateInfo(ShaderType _type, const std::wstring& _filePath, const Char* _entryPoint,
			const void* _byteCode = nullptr, size_t _byteCodeSize = 0)
			: Type(_type)
			, FilePath(_filePath)
			, EntryPoint(_entryPoint)
			, ByteCode(_byteCode)
			, ByteCodeSize(_byteCodeSize)
		{}
	};
	typedef struct ShaderCreateInfo ShaderCreateInfo;

	class Shader {
	public:
		KRS_CLASS_DEFAULT(Shader);

		Shader(class RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3DBlob> blob);

		ID3DBlob* GetD3DBlob() { return m_ShaderBlob.Get(); }
	private:
		class RenderDevice* m_Device;

		Microsoft::WRL::ComPtr<ID3DBlob> m_ShaderBlob;
		D3D12_SHADER_BYTECODE m_ByteCode;

	};
	

	class ShaderManager {
	public:
		KRS_CLASS_NON_COPYABLE_AND_MOVABLE(ShaderManager);
		ShaderManager(RenderDevice* pDevice, const std::filesystem::path& workingDirectory);
		
		Ref<Shader> LoadShader(ShaderType type, const std::string& entryPoint, const std::string& relativePath);
		
	
	private:
		String BuildShaderProfile(ShaderType type);


		RenderDevice* m_Device;
		std::vector<Ref<Shader>> m_ShaderCache;

		std::filesystem::path m_ExecutablePath;
		std::filesystem::path m_ShaderSrcPath;
	};
}