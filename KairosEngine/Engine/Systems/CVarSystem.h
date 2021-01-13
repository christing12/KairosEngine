#pragma once

#include "Interface/ICVarSystem.h"

KRS_BEGIN_NAMESPACE(Kairos)

class CVarSystem : public ICVarSystem {
public:

	virtual bool Setup(ISystemConfig* config = nullptr) override final;
	virtual bool Init() override final;
	virtual bool Update() override final;
	virtual bool Shutdown() override final;

	virtual CVarParameter* GetCVar(StringUtils::StringHash hash) override;
	virtual double* GetFloatCVar(StringUtils::StringHash has) override;

	virtual int32_t* GetIntCVar(StringUtils::StringHash hash) override;
	virtual const char* GetStringCVar(StringUtils::StringHash hash) override;
	virtual void SetFloatCVar(StringUtils::StringHash hash, double value) override;


	virtual void SetIntCVar(StringUtils::StringHash hash, int32_t value) override;

	virtual void SetStringCVar(StringUtils::StringHash hash, const char* value) override;


	virtual CVarParameter* CreateFloatCVar(const char* name, const char* description, double defaultValue, double currentValue) override;

	virtual CVarParameter* CreateIntCVar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue) override;

	virtual CVarParameter* CreateStringCVar(const char* name, const char* description, const char* defaultValue, const char* currentValue) override;

	virtual void DrawImguiEditor() override;
};


KRS_END_NAMESPACE