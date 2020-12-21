#include "krspch.h"
#include "Shader.h"
#include "RenderDevice.h"

namespace Kairos {
	Shader::Shader(RenderDevice* pDevice, const ShaderCreateInfo& createInfo)
		: m_Device(pDevice)
	{
		ID3D12Device2* pdDevice = pDevice->GetD3DDevice();

		Shader::CompileFromFile(createInfo, m_ShaderBlob.ReleaseAndGetAddressOf());
		//spdDevice->Createsha
	}
	/* static */ void Shader::CompileFromFile(const ShaderCreateInfo& info, ID3DBlob** ppBlob)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* pErrorBlob = nullptr;


		std::string profile = BuildShaderProfile(info.Type);

		auto hr = D3DCompileFromFile(info.FilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			info.EntryPoint, profile.c_str(), dwShaderFlags, 0, ppBlob, &pErrorBlob);

		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with compiling shader from file");
	}

	String Shader::BuildShaderProfile(ShaderType type)
	{
		String profile;
		switch (type)
		{
		case ShaderType::Vertex:        profile = "vs"; break;
		case ShaderType::Pixel:         profile = "ps"; break;
		default: KRS_CORE_WARN("Unknown Shader Type");
		}
		profile += "_5_1";
		return profile;
	}
}