#pragma once

namespace Kairos {
	enum class ShaderType : Uint32 {
		Vertex = 0,
		Pixel = Vertex + 1,
		NumShaderTypes
	};

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
		Shader() {}
		Shader(class RenderDevice* pDevice, const ShaderCreateInfo& createInfo);

		ID3DBlob* GetD3DBlob() { return m_ShaderBlob.Get(); }
	private:
		static void CompileFromFile(const ShaderCreateInfo& info, ID3DBlob** ppBlob);
		Microsoft::WRL::ComPtr<ID3DBlob> m_ShaderBlob;
		D3D12_SHADER_BYTECODE m_ByteCode;
		static String BuildShaderProfile(ShaderType type);

		class RenderDevice* m_Device;
	};
}