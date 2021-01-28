#include "krspch.h"
#include "EnvironmentMap.h"
#include "RenderBackend/RHI/PipelineStateManager.h"
#include "RenderBackend/RHI/Texture.h"
#include "RenderBackend/RHI/ResourceFormat.h"
#include "RenderBackend/RHI/RenderDevice.h"
#include "RenderBackend/RHI/CommandContext.h"

namespace Kairos {
	void EnvironmentMap::Init(RenderDevice* pDevice, PipelineStateManager* manager)
	{
		manager->CreateRootSignature("Default", [](RootSignatureProxy& proxy) {});
		manager->CreateRootSignature("OneTexture", [](RootSignatureProxy& proxy)
			{
				proxy.AddRootConstant(1, 0, 0);
			});
		manager->CreateRootSignature("TwoTexture", [](RootSignatureProxy& proxy)
			{
				proxy.AddRootConstant(2, 0, 0);
			});

		auto result = manager->CreateComputePipelineState("Equirect2Cube", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "equirect2cube.hlsl";
				proxy.RootSignatureName = "TwoTexture";
			});
		result.Pipeline->Finalize();

		result = manager->CreateComputePipelineState("brdfLUT", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "spbrdf.hlsl";
				proxy.RootSignatureName = "OneTexture";
			});
		result.Pipeline->Finalize();

		result = manager->CreateComputePipelineState("spmap", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "spmap.hlsl";
				proxy.RootSignatureName = "OneTexture";
			});
		result.Pipeline->Finalize();
		
		result = manager->CreateComputePipelineState("irMap", [](ComputePiplineProxy& proxy)
			{
				proxy.CSFile = "irmap.hlsl";
				proxy.RootSignatureName = "TwoTexture";
			});
		result.Pipeline->Finalize();



		m_envTexture = CreateScope<Texture>(pDevice, TextureProperties(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, TextureType::Tex2D, 6, 1), ResourceState::UnorderedAccess);
		
		Texture* unfilteredEnv = new Texture(
			pDevice, 
			TextureProperties(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, TextureType::Tex2D, 6, 1),
			ResourceState::UnorderedAccess
		);
		


		m_equirectTex = pDevice->CreateTexture("Data/textures/assets_export/environment.tx", DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1);

		// UP until here is correct

			//pDevice->CreateTexture(, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1);
		struct EquiStruct {
			Uint32 inputTextureIndex;
			Uint32 outputTextureIndex;
		};
		EquiStruct instance{ m_equirectTex->GetSRDescriptor().Index(), m_envTexture->GetUAVDescriptor(0).Index() };

		//DynamicBuffer* constants = reinterpret_cast<DynamicBuffer*>(pDevice->CreateDynConstBuffer(sizeof(EquiStruct), sizeof(EquiStruct)));
		//memcpy(constants->DataPtr(), &instance, sizeof(instance));
		// Convert equirectangular HDR image into textureCube cubemap
		pDevice->TransientCommand([&](CommandContext& ctx)
		{
			ctx.TransitionResource(*m_equirectTex, ResourceState::NonPixelAccess, true);


			auto& computeCtx = ctx.GetComputeContext();
			computeCtx.SetPipelineState("Equirect2Cube");
			computeCtx.SetRootConstants(0, 0, 2, &instance);
			computeCtx.Dispatch(m_envTexture->Width() / 32, m_envTexture->Height() / 32, 6);
			computeCtx.TransitionResource(*m_envTexture, ResourceState::NonPixelAccess);
		}, CommandType::Compute);

		//// for specular-roughness map(right now only 1st mip level so jsut copy the result of the equirect2cube
		//pDevice->TransientCommand([&](CommandContext& ctx)
		//	{
		//		ctx.CopyTextureRegion(m_envTexture.get(), unfilteredEnv, 0, 0, 0);
		//		ctx.TransitionResource(*m_envTexture, ResourceState::UnorderedAccess);
		//		ctx.TransitionResource(*unfilteredEnv, ResourceState::NonPixelAccess);
		//	});


		m_irradianceTexture = CreateScope<Texture>(pDevice,
			TextureProperties(32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, TextureType::Tex2D, 6, 1),
			ResourceState::UnorderedAccess
		);
		// compute irradiance map
		pDevice->TransientCommand([&](CommandContext& ctx)
		{
			
			auto& computeCtx = ctx.GetComputeContext();			
			struct irmapInput {
				Uint32 inputTextureIndex;
				Uint32 outputTextureIndex;
			};
			computeCtx.SetPipelineState("irMap");
			irmapInput instance{ m_envTexture->GetSRDescriptor().Index(), m_irradianceTexture->GetUAVDescriptor(0).Index() };
			computeCtx.SetRootConstants(0, 0, 2, &instance);
			computeCtx.Dispatch(1, 1, 6);
			computeCtx.TransitionResource(*m_irradianceTexture, ResourceState::Common);

		}, CommandType::Compute);



		m_specularBRDF = CreateScope<Texture>(pDevice,
			TextureProperties(256, 256, DXGI_FORMAT_R16G16_FLOAT, TextureType::Tex2D, 1, 1),
			ResourceState::UnorderedAccess
		);
		pDevice->TransientCommand([&](CommandContext& ctx)
			{
				ComputeContext& compute = ctx.GetComputeContext();
				compute.SetPipelineState("brdfLUT");
				auto index = m_specularBRDF->GetUAVDescriptor(0).Index();
				compute.SetRootConstants(0, 0, 1, &index);
				compute.Dispatch(m_specularBRDF->Width() / 32, m_specularBRDF->Height() / 32, 1);
				compute.TransitionResource(*m_specularBRDF, ResourceState::Common);
			}, CommandType::Compute);

		delete unfilteredEnv;
	}

}

