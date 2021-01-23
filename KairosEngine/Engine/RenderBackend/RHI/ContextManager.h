#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include "GraphicsTypes.h"

#define CONTEXT_TYPES 4

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class CommandContext;
class CommandQueue;

class ContextManager {
	friend class RenderDevice;
public:

	ContextManager() = default;
	~ContextManager() = default;

	CommandContext* AllocateContext(RenderDevice* pDevice, CommandType type, CommandQueue& commandQueue);

	void FreeContext(CommandContext* context);
	void Shutdown();
private:
	using ContextPool = std::vector<Ref<class CommandContext>>;
	using FreedContexts = std::queue<CommandContext*>;

	ContextPool m_ContextPool[CONTEXT_TYPES];
	FreedContexts m_FreeContexts[CONTEXT_TYPES];

	std::mutex m_Mutex;
};


KRS_END_NAMESPACE