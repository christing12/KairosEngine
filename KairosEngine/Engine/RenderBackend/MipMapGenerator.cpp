#include "krspch.h"
#include "MipMapGenerator.h"

#include "RenderBackend/RenderDevice.h"
#include "RenderBackend/PipelineState.h"
#include "RenderBackend/RootSignature.h"
#include "RenderBackend/Texture.h"
#include "RenderBackend/Resource.h"
#include "RenderBackend/Shader.h"
#include "RenderBackend/CommandContext.h"

#include "Systems/Interface/IEngine.h"
#include "Systems/Interface/IRenderBackend.h"

extern Kairos::IEngine* g_Engine;

namespace Kairos {
	MipMapGenerator::MipMapGenerator(RenderDevice* pDevice)
		: m_Device(pDevice)
	{
		//m_Signature = g_Engine->GetRenderBackend()->AllocateRootSignature("MipMap", 2, 1);
		(*m_Signature)[0].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_ALL);
		(*m_Signature)[1].InitAsDescriptorTable(1, D3D12_SHADER_VISIBILITY_ALL);
		(*m_Signature)[0].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		(*m_Signature)[1].SetDescriptorRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

		m_Signature->Finalize(L"Mip Map Generator Sig", D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);



	}
	void MipMapGenerator::GenerateMipMap(const Texture& texture)
	{
		std::wstring path = L"C:/Users/Chris Ting/Desktop/MeshRenderer/KairosEngine/Engine/Graphics/Shaders/";

		Ref<PipelineState> pipelineState;

		const D3D12_RESOURCE_DESC desc = texture.GetDesc();
		if (desc.DepthOrArraySize == 1 && desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
			if (m_GammaPipeline == nullptr) {
				Ref<Shader> shader = CreateRef<Shader>(m_Device, ShaderCreateInfo(ShaderType::Compute, path + L"downsample.hlsl", "downsample_gamma"));

				m_GammaPipeline = CreateRef<ComputePSO>(m_Device);
				m_GammaPipeline->SetRootSignature(*m_Signature);
				m_GammaPipeline->SetComputeShader(shader);
				m_GammaPipeline->Finalize();
			}
		}
		else if (desc.DepthOrArraySize > 1 && desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
			if (m_ArrayPipeline == nullptr) {
				Ref<Shader> shader = CreateRef<Shader>(m_Device, ShaderCreateInfo(ShaderType::Compute, path + L"downsample_array.hlsl", "downsample_linear"));

				m_ArrayPipeline = CreateRef<ComputePSO>(m_Device);
				m_ArrayPipeline->SetRootSignature(*m_Signature);
				m_ArrayPipeline->SetComputeShader(shader);
				m_ArrayPipeline->Finalize();
			}
		}
		else  {
			if (m_LinearPipeline == nullptr) {
				Ref<Shader> shader = CreateRef<Shader>(m_Device, ShaderCreateInfo(ShaderType::Compute, path + L"downsample.hlsl", "downsample_linear"));

				m_LinearPipeline = CreateRef<ComputePSO>(m_Device);
				m_LinearPipeline->SetRootSignature(*m_Signature);
				m_LinearPipeline->SetComputeShader(shader);
				m_LinearPipeline->Finalize();
			}
		}

		//ComputeContext& context = m_Device->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetComputeContext();

		//Texture linearTexture = texture;

		//if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
		//	pipelineState = m_GammaPipeline;


		//	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, texture.Width(), texture.Height(), 1, texture.GetDesc().MipLevels);
		//	linearTexture = Texture(m_Device, desc, D3D12_RESOURCE_STATE_COMMON);

		//	context.TransitionResource(linearTexture, D3D12_RESOURCE_STATE_COPY_DEST);
		//	context.CopyResource(linearTexture, texture);
		//	context.TransitionResource(linearTexture, D3D12_RESOURCE_STATE_COMMON);
		//}

		//context.SetRootSignature(*m_Signature);
		////context.SetDynamicDescriptors()
		//context.SetPipelineState(*pipelineState);

		//
		//for (Uint32 level = 1, levelWidth = texture.Width() / 2, levelHeight = texture.Height() / 2; level < texture.GetDesc().MipLevels;
		//	++level, levelWidth /= 2, levelHeight /= 2) {

		//	for (Uint32 arraySlice = 0; arraySlice < desc.DepthOrArraySize; ++arraySlice) {

		//	}

		//}

	}
}