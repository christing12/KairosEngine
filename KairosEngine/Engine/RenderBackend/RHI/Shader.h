#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)

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

KRS_END_NAMESPACE