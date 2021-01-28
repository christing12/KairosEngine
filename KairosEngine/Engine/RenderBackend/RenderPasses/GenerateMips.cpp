#include "krspch.h"
#include "GenerateMips.h"
#include "RenderBackend/RHI/RenderDevice.h"
#include "RenderBackend/RHI/PipelineStateManager.h"
#include "RenderBackend/RHI/Texture.h"
#include "RenderBackend/RHI/CommandContext.h"
#include "RenderBackend/RHI/DX12Util.h"

namespace Kairos {
	void GenerateMips::Init(RenderDevice* pDevice, PipelineStateManager* manager)
	{
		m_Device = pDevice;
		manager->CreateRootSignature("TwoTexture", [](RootSignatureProxy& proxy)
			{
				proxy.AddRootConstant(2, 0, 0);
			});

		auto result = manager->CreateComputePipelineState("MipsLinear", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "downsample.hlsl";
				proxy.RootSignatureName = "TwoTexture";
				proxy.EntryPoint = "downsample_linear";
			});
		result.Pipeline->Finalize();

		result = manager->CreateComputePipelineState("MipsGamma", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "downsample.hlsl";
				proxy.RootSignatureName = "TwoTexture";
				proxy.EntryPoint = "downsample_gamma";
			});
		result.Pipeline->Finalize();
		result = manager->CreateComputePipelineState("MipsArrayLinear", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "downsample.hlsl";
				proxy.RootSignatureName = "TwoTexture";
				proxy.EntryPoint = "downsample_array_linear";
			});
		result.Pipeline->Finalize();
	}

	void GenerateMips::Generate(Texture* inputTexture)
	{
		KRS_CORE_ASSERT(inputTexture->Width() == inputTexture->Height(), "Not an even texture");
		KRS_CORE_ASSERT(Math::isPowerOfTwo(inputTexture->Width()), "Not a power of two");

		ComputeContext& ctx = m_Device->AllocateCommandContext(CommandType::Compute).GetComputeContext();

		if (inputTexture->Depth() == 1 & inputTexture->GetDesc().Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
			ctx.SetPipelineState("MipsGamma");
		else if (inputTexture->Depth() > 1 && inputTexture->GetDesc().Format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
			ctx.SetPipelineState("MipsArrayLinear");
		else
			ctx.SetPipelineState("MipsLinear");

		Texture* texturePtr = inputTexture;
		if (inputTexture->GetDesc().Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			texturePtr = new Texture(m_Device,
				TextureProperties(inputTexture->Width(), inputTexture->Height(), DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D,
					inputTexture->Depth(), inputTexture->MipLevels()), ResourceState::Common);
			m_Device->TransientCommand([&](CommandContext& ctx)
				{
					ctx.TransitionResource(*texturePtr, ResourceState::CopyDest, true);
					ctx.CopyResource(*texturePtr, *inputTexture);
					ctx.TransitionResource(*texturePtr, ResourceState::Common);
				});

		}

		ctx.TransitionResource(*texturePtr, ResourceState::UnorderedAccess, true);
		DescriptorRange alloc = m_Device->AllocateDescriptor(DescriptorType::SRV, inputTexture->MipLevels(), true);
		for (Uint32 level = 1; level < inputTexture->MipLevels(); ++level)
		{
			Uint32 levelWidth = inputTexture->Width() / (2 * level);
			Uint32 levelHeight = inputTexture->Height() / (2 * level);

			Descriptor srv = alloc.GetDescriptor(level);
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = DX12Util::GetSRVDesc(texturePtr->GetDesc());
			m_Device->D3DDevice()->CreateShaderResourceView(texturePtr->D3DResource(), &desc, srv.CPUHandle());

			Descriptor uav = texturePtr->GetUAVDescriptor(level);


			Uint32 data[] = {
				srv.Index(),
				uav.Index()
			};
			ctx.SetRootConstants(0, 0, 2, &data[0]);
			ctx.Dispatch(Math::Max(Uint32(1), levelWidth / 8), Math::Max(Uint32(1), levelHeight / 8), texturePtr->Depth());
		}
		ctx.TransitionResource(*texturePtr, ResourceState::Common, true);

		//Texture tempTex = *inputTexture;

	}

}
