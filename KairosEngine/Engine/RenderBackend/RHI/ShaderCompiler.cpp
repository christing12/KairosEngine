#include "krspch.h"
#include "ShaderCompiler.h"
#include "GraphicsTypes.h"


namespace Kairos {

	ShaderCompiler::ShaderCompiler()
	{
		auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(m_Utils.ReleaseAndGetAddressOf()));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "DXC LIbrary cannot be created");

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_Compiler.ReleaseAndGetAddressOf()));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "DXC Compiler cannot be created");


	}

	ShaderCompileResult ShaderCompiler::CompileShader(const std::filesystem::path& path, std::string entryPoint, ShaderType type)
	{
		ComPtr<IDxcBlobEncoding> pSource;

		Uint32 codePage = CP_UTF8;

		std::wstring wfile = Filesystem::StringToWString(path.string());
		std::wstring wentry = Filesystem::StringToWString(entryPoint);
		std::wstring wprofile = Filesystem::StringToWString(ShaderProfile(type));

		auto hr = m_Utils->CreateBlobFromFile(wfile.c_str(), &codePage, &pSource);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue finding file");


		std::vector<std::wstring> arguments;
		arguments.push_back(L"-Zpr"); // row major matrices
		arguments.push_back(L"-WX"); // warnings as errors
#ifdef _DEBUG
		arguments.push_back(L"-Zi"); // debug info
		arguments.push_back(L"-Qembed_debug"); // embed debug info
		arguments.push_back(L"-Od"); // disable optimizations
#else
		arguments.push_back(L"-O3"); // op level 3
#endif

		std::vector<LPCWSTR> argumentPtrs;
		for (auto& arg : arguments)
		{
			argumentPtrs.push_back(arg.c_str());
		}

		ComPtr<IDxcIncludeHandler> includeHandler;
		m_Utils->CreateIncludeHandler(&includeHandler);

		ComPtr<IDxcOperationResult> result;
		hr = m_Compiler->Compile(
			pSource.Get(),
			wfile.c_str(),
			wentry.c_str(),
			wprofile.c_str(),
			argumentPtrs.data(), argumentPtrs.size(), // arguments, arg Count
			nullptr, 0,
			includeHandler.Get(),
			&result
		);

		if (SUCCEEDED(hr))
			result->GetStatus(&hr);
		if (FAILED(hr))
		{
			if (result)
			{
				ComPtr<IDxcBlobEncoding> errorsBlob;
				hr = result->GetErrorBuffer(&errorsBlob);
				if (SUCCEEDED(hr) && errorsBlob)
				{
					KRS_CORE_INFO(path.generic_string());
					KRS_CORE_INFO(entryPoint);
					KRS_CORE_ASSERT(false, (const char*)errorsBlob->GetBufferPointer());
				}
			}
		}
		ComPtr<IDxcBlob> code;
		result->GetResult(&code);

		return ShaderCompileResult{ code };
	}

	std::string ShaderCompiler::ShaderProfile(ShaderType type)
	{
		std::string profile;
		switch (type)
		{
		case ShaderType::Vertex:        profile = "vs"; break;
		case ShaderType::Pixel:         profile = "ps"; break;
		case ShaderType::Compute:		profile = "cs"; break;
		default:
			KRS_CORE_WARN("Unknown Shader Type");
		}

		profile += "_6_3";

		return profile;
	}


}

