#include "krspch.h"
#include "ContextManager.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "CommandQueue.h"

namespace Kairos {
	CommandContext* ContextManager::AllocateContext(RenderDevice* pDevice, CommandType type, CommandQueue& commandQueue)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		auto& contextOfType = m_FreeContexts[static_cast<size_t>(type)];
		CommandContext* pCommandContext;

		if (!contextOfType.empty()) {
			pCommandContext = contextOfType.front();
			contextOfType.pop();
			pCommandContext->Reset(&pDevice->GetCommandQueue(type));
		}
		else {
			pCommandContext = new CommandContext(pDevice, type);
			m_ContextPool[static_cast<size_t>(type)].emplace_back(pCommandContext);
		}
		return pCommandContext;
	}
	void ContextManager::FreeContext(CommandContext* pContext)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_FreeContexts[static_cast<size_t>(pContext->GetType())].push(pContext);
	}

	void ContextManager::Shutdown()
	{
		for (size_t i = 0; i < 4; ++i)
			m_ContextPool[i].clear();
	}

}