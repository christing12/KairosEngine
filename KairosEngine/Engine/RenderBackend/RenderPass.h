#pragma once


#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>



KRS_BEGIN_NAMESPACE(Kairos)

class CommandContext;
class ResourceStorage;
class PipelineStateManager;
class RenderDevice;

class RenderPass {
public:
	RenderPass() = default;
	RenderPass(const std::string& _name) : m_Name(_name) {}
	virtual ~RenderPass() {}

	virtual void Setup(RenderDevice* pDevice, Kairos::PipelineStateManager* manager) = 0;
	virtual void Execute(Kairos::CommandContext& context, Kairos::ResourceStorage& data, CPVoid constantBufferData = nullptr, Uint32 dataSize = 0) = 0;

protected:
	RenderDevice* m_Device;
	std::string m_Name;

public:
	inline const std::string& Name() const { return m_Name; }
};


KRS_END_NAMESPACE