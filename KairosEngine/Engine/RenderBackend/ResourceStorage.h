#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "RenderBackend/RHI/Buffer.h"

KRS_BEGIN_NAMESPACE(Kairos)

class Buffer;
class DynamicBuffer;
class RenderDevice;

// TODO: more functionality
struct RenderPassData
{
	Scope<DynamicBuffer> ConstantBufferData = nullptr;
};


class ResourceStorage {
public:
	using RenderPassMap = std::unordered_map<std::string, RenderPassData>;
	ResourceStorage() = default;
	ResourceStorage(RenderDevice* pDevice);

	template<class ConstantType>
	void UpdatePassConstants(const ConstantType& data, const std::string& passName);


	void AddPass(const std::string& name);

private:
	RenderDevice* m_Device;
	RenderPassData& GetPassData(const std::string& name);

	RenderPassMap m_RenderPassData;
};

template<class ConstantType>
void ResourceStorage::UpdatePassConstants(const ConstantType& data, const std::string& passName)
{
	auto& passData = GetPassData(passName);
	KRS_CORE_ASSERT(sizeof(ConstantType) < passData.ConstantBufferData->Size(), "Not enough space in dynamic buffer to store new data");
	if (passData.ConstantBufferData == nullptr)
	{
		passData.ConstantBufferData = CreateScope<DynamicBuffer>(DynamicBuffer::Create(m_Device, BufferProperties(sizeof(ConstantType), sizeof(ConstantType), 0)));
	}
	passData.ConstantBufferData->Write(data);
}


KRS_END_NAMESPACE