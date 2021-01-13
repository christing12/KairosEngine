#include "krspch.h"
#include "Shader.h"
#include "RenderDevice.h"
#include "GraphicsTypes.h"

namespace Kairos {
	Shader::Shader(RenderDevice* pDevice, ComPtr<ID3DBlob> blob)
		: m_Device(pDevice)
		, m_ShaderBlob(blob)
	{
		m_ByteCode = CD3DX12_SHADER_BYTECODE(blob.Get());
	}




	ShaderManager::ShaderManager(RenderDevice* pDevice, const std::filesystem::path& workingDirectory)
		: m_Device(pDevice)
		, m_ExecutablePath(workingDirectory)
	{
		m_ShaderSrcPath = m_ExecutablePath / "Data/shaders";
	}

	
	Ref<Shader> ShaderManager::LoadShader(ShaderType type, const std::string& entryPoint, const std::string& relativePath)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ComPtr<ID3DBlob> pErrorBlob = nullptr;
		ComPtr<ID3DBlob> outputBlob = nullptr;

		std::string profile = BuildShaderProfile(type);

		auto hr = D3DCompileFromFile((m_ShaderSrcPath / relativePath).c_str() , nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(), profile.c_str(), dwShaderFlags, 0, &outputBlob, &pErrorBlob);

		KRS_CORE_ASSERT(SUCCEEDED(hr), ((char*)pErrorBlob->GetBufferPointer()));

		Ref<Shader> shader = CreateRef<Shader>(m_Device, outputBlob);
		m_ShaderCache.emplace_back(shader);
		return shader;
	}



	String ShaderManager::BuildShaderProfile(ShaderType type)
	{
		String profile;
		switch (type)
		{
		case ShaderType::Vertex:        profile = "vs"; break;
		case ShaderType::Pixel:         profile = "ps"; break;
		case ShaderType::Compute:		profile = "cs"; break;
		default: KRS_CORE_WARN("Unknown Shader Type");
		}
		if (type != ShaderType::Compute) {
			profile += "_5_1";

		}
		else {
			profile += "_5_0";
		}
		return profile;
	}
}