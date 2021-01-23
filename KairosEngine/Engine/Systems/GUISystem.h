#pragma once

#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>
#include <Core/ClassTemplates.h>


KRS_BEGIN_NAMESPACE(Kairos)

class GUISystem {
public:
	GUISystem() = default;
	~GUISystem() = default;

	bool Setup(struct ISystemConfig* config = nullptr);
	bool Init();
	bool Update();
	bool Shutdown();

	virtual void Render(class GraphicsContext& context);

private:
	void InitStyle();

	bool m_ShowImGui = true;
	std::function<void()> m_ShowImGuiFn;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_GUIHeap;
};


KRS_END_NAMESPACE