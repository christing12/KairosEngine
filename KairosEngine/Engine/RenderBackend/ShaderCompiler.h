#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>
#include <filesystem>

KRS_BEGIN_NAMESPACE(Kairos)

struct ShaderCompileResult
{
    Microsoft::WRL::ComPtr<IDxcBlob> Code;
};

class ShaderCompiler {
public:
    ShaderCompiler();

    ShaderCompileResult CompileShader(const std::filesystem::path& path, std::string entryPoint, ShaderType type);
private:
    std::string ShaderProfile(ShaderType type);

    Microsoft::WRL::ComPtr<IDxcLibrary> m_Utils;
    Microsoft::WRL::ComPtr<IDxcCompiler> m_Compiler;
};

KRS_END_NAMESPACE