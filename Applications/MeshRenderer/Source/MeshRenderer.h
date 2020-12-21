
#pragma once

#include <KairosAPI.h>


class MeshRenderer : public Kairos::Application {
public:
	MeshRenderer();
	virtual ~MeshRenderer() = default;

	virtual void InitEngine() override final;
	virtual void Render() override final;
	virtual void Update(float deltaTime) override final;
	virtual void Present() override final;

private:
};

Kairos::Application* Kairos::CreateApplication() {
	return new MeshRenderer();
}