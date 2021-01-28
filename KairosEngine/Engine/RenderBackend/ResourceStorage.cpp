#include "krspch.h"
#include "ResourceStorage.h"

namespace Kairos {
	ResourceStorage::ResourceStorage(RenderDevice* pDevice)
		: m_Device(pDevice)
	{
	}
	void ResourceStorage::AddPass(const std::string& name)
	{
		auto it = m_RenderPassData.find(name);
		if (it != m_RenderPassData.end()) {
			KRS_CORE_ERROR("Pass has already been registered");
			return;
		}
		
		m_RenderPassData[name] = RenderPassData{};
	}

	RenderPassData& ResourceStorage::GetPassData(const std::string& name)
	{
		auto iter = m_RenderPassData.find(name);
		if (iter != m_RenderPassData.end())
		{
			return iter->second;
		}
	}


}

