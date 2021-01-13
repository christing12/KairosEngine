#include "krspch.h"
#include "CVarSystem.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

namespace Kairos {
	enum class CVarType : char
	{
		INT,
		FLOAT,
		STRING,
	};

	class CVarParameter
	{
	public:
		int32_t arrayIndex;

		CVarType type;
		CVarFlags flags;
		std::string name;
		std::string description;
	};


	template<typename T>
	struct CVarStorage {
		T initial;
		T current;
		CVarParameter* parameter;
	};

	template<typename T>
	struct CVarArray {
		CVarStorage<T>* cvars;
		int32_t lastCVar{ 0 };
		CVarArray(size_t size) {
			cvars = new CVarStorage<T>[size]();
		}
	/*	~CVarArray() {
			delete cvars;
		}*/

		CVarStorage<T>* GetCurrentStorage(int32_t index)
		{
			return &cvars[index];
		}

		T* GetCurrentPtr(int32_t index)
		{
			return &cvars[index].current;
		};

		T GetCurrent(int32_t index) {
			return cvars[index].current;
		}

		void SetCurrent(const T& val, int32_t index) {
			cvars[index].current = val;
		}

		int Add(const T& value, CVarParameter* param) {
			int index = lastCVar;
			cvars[index].current = value;
			cvars[index].initial = value;
			cvars[index].parameter = param;

			param->arrayIndex = index;
			lastCVar++;
			return index;
		}

		int Add(const T& initValue, const T& currVal, CVarParameter* param) {
			int index = lastCVar;
			cvars[index].initial = initValue;
			cvars[index].current = currVal;
			cvars[index].parameter = param;

			param->arrayIndex = index;
			lastCVar++;
			return index;
		}
	};

	namespace CVarSystemImpl {
		constexpr static int MAX_INT_CVARS = 1000;
		CVarArray<int32_t> intCVars2{ MAX_INT_CVARS };

		constexpr static int MAX_FLOAT_CVARS = 1000;
		CVarArray<double> floatCVars{ MAX_FLOAT_CVARS };

		constexpr static int MAX_STRING_CVARS = 200;
		CVarArray<std::string> stringCVars{ MAX_STRING_CVARS };


		CVarParameter* GetCVar(StringUtils::StringHash hash);
		double* GetFloatCVar(StringUtils::StringHash has);

		int32_t* GetIntCVar(StringUtils::StringHash hash);
		const char* GetStringCVar(StringUtils::StringHash hash);
		void SetFloatCVar(StringUtils::StringHash hash, double value);


		void SetIntCVar(StringUtils::StringHash hash, int32_t value);

		void SetStringCVar(StringUtils::StringHash hash, const char* value);


		CVarParameter* CreateFloatCVar(const char* name, const char* description, double defaultValue, double currentValue) ;

		CVarParameter* CreateIntCVar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue) ;

		CVarParameter* CreateStringCVar(const char* name, const char* description, const char* defaultValue, const char* currentValue) ;

		void DrawImguiEditor();
		void EditParameter(CVarParameter* p, float textWidth);



		template<typename T>
		CVarArray<T>* GetCVarArray();

		template<>
		CVarArray<int32_t>* GetCVarArray()
		{
			return &intCVars2;
		}

		template<>
		CVarArray<double>* GetCVarArray() {
			return &floatCVars;
		}

		template<>
		CVarArray<std::string>* GetCVarArray() {
			return &stringCVars;
		}

		template<typename T>
		T* GetCVarCurrent(uint32_t namehash) {
			CVarParameter* par = GetCVar(namehash);
			if (!par)
				return nullptr;
			else
				return GetCVarArray<T>()->GetCurrentPtr(par->arrayIndex);
		}

		template<typename T>
		void SetCVarCurrent(uint32_t namehash, const T& value) {
			CVarParameter* cvar = GetCVar(namehash);
			if (cvar)
				GetCVarArray<T>()->SetCurrent(value, cvar->arrayIndex);
		}

		std::shared_mutex mutex;

		CVarParameter* InitCVar(const char* name, const char* description);
		std::unordered_map<uint32_t, CVarParameter> savedCVars;
		std::vector<CVarParameter*> cachedEditParameters;

	}

	// -------- GETTERS -------------- //
	CVarParameter* CVarSystemImpl::GetCVar(StringUtils::StringHash hash)
	{
		std::shared_lock lock(mutex);
		auto it = savedCVars.find(hash);
		if (it != savedCVars.end())
			return &(*it).second;

		return nullptr;
	}


	double* CVarSystemImpl::GetFloatCVar(StringUtils::StringHash hash)
	{
		return GetCVarCurrent<double>(hash);
	}
	int32_t* CVarSystemImpl::GetIntCVar(StringUtils::StringHash hash)
	{
		return GetCVarCurrent<int32_t>(hash);
	}
	const char* CVarSystemImpl::GetStringCVar(StringUtils::StringHash hash)
	{
		return GetCVarCurrent<std::string>(hash)->c_str();
	}

	// --- SETTERS ------------/

	void CVarSystemImpl::SetFloatCVar(StringUtils::StringHash hash, double value)
	{
		SetCVarCurrent<double>(hash, value);
	}
	void CVarSystemImpl::SetIntCVar(StringUtils::StringHash hash, int32_t value)
	{
		SetCVarCurrent<int32_t>(hash, value);
	}
	void CVarSystemImpl::SetStringCVar(StringUtils::StringHash hash, const char* value)
	{
		SetCVarCurrent<std::string>(hash, value);
	}


	CVarParameter* CVarSystemImpl::CreateFloatCVar(const char* name, const char* description, double defaultValue, double currentValue)
	{
		std::unique_lock lock(mutex);
		CVarParameter* param = InitCVar(name, description);
		if (!param) return nullptr;
		param->type = CVarType::FLOAT;

		GetCVarArray<double>()->Add(defaultValue, currentValue, param);

		return param;
	}


	CVarParameter* CVarSystemImpl::CreateIntCVar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue)
	{
		std::unique_lock lock(mutex);
		CVarParameter* param = InitCVar(name, description);
		if (!param) return nullptr;
		param->type = CVarType::INT;

		GetCVarArray<int32_t>()->Add(defaultValue, currentValue, param);

		return param;
	}


	CVarParameter* CVarSystemImpl::CreateStringCVar(const char* name, const char* description, const char* defaultValue, const char* currentValue)
	{
		std::unique_lock lock(mutex);
		CVarParameter* param = InitCVar(name, description);
		if (!param) return nullptr;
		param->type = CVarType::STRING;

		GetCVarArray<std::string>()->Add(defaultValue, currentValue, param);

		return param;
	}

	CVarParameter* CVarSystemImpl::InitCVar(const char* name, const char* description)
	{
		uint32_t namehash = StringUtils::StringHash{ name };
		savedCVars[namehash] = CVarParameter{};

		CVarParameter& newParam = savedCVars[namehash];
		newParam.name = name;
		newParam.description = description;

		return &newParam;
	}







	AutoCVar_Float::AutoCVar_Float(const char* name, const char* description, double defaultValue, CVarFlags flags)
	{
		CVarParameter* cvar = CVarSystemImpl::CreateFloatCVar(name, description, defaultValue, defaultValue);
		cvar->flags = flags;
		index = cvar->arrayIndex;
	}

	template<typename T>
	T GetCVarCurrentByIndex(int32_t index) {
		return CVarSystemImpl::GetCVarArray<T>()->GetCurrent(index);
	}
	template<typename T>
	T* PtrGetCVarCurrentByIndex(int32_t index) {
		return CVarSystemImpl::GetCVarArray<T>()->GetCurrentPtr(index);
	}


	template<typename T>
	void SetCVarCurrentByIndex(int32_t index, const T& data) {
		CVarSystemImpl::GetCVarArray<T>()->SetCurrent(data, index);
	}


	double AutoCVar_Float::Get()
	{
		return GetCVarCurrentByIndex<CVarType>(index);
	}

	double* AutoCVar_Float::GetPtr()
	{
		return PtrGetCVarCurrentByIndex<CVarType>(index);
	}

	float AutoCVar_Float::GetFloat()
	{
		return static_cast<float>(Get());
	}

	float* AutoCVar_Float::GetFloatPtr()
	{
		float* result = reinterpret_cast<float*>(GetPtr());
		return result;
	}

	void AutoCVar_Float::Set(double f)
	{
		SetCVarCurrentByIndex<CVarType>(index, f);
	}

	AutoCVar_Int::AutoCVar_Int(const char* name, const char* description, int32_t defaultValue, CVarFlags flags)
	{
		CVarParameter* cvar = CVarSystemImpl::CreateIntCVar(name, description, defaultValue, defaultValue);
		cvar->flags = flags;
		index = cvar->arrayIndex;
	}

	int32_t AutoCVar_Int::Get()
	{
		return GetCVarCurrentByIndex<CVarType>(index);
	}

	int32_t* AutoCVar_Int::GetPtr()
	{
		return PtrGetCVarCurrentByIndex<CVarType>(index);
	}

	void AutoCVar_Int::Set(int32_t val)
	{
		SetCVarCurrentByIndex<CVarType>(index, val);
	}

	void AutoCVar_Int::Toggle()
	{
		bool enabled = Get() != 0;

		Set(enabled ? 0 : 1);
	}

	AutoCVar_String::AutoCVar_String(const char* name, const char* description, const char* defaultValue, CVarFlags flags)
	{
		CVarParameter* cvar = CVarSystemImpl::CreateStringCVar(name, description, defaultValue, defaultValue);
		cvar->flags = flags;
		index = cvar->arrayIndex;
	}

	const char* AutoCVar_String::Get()
	{
		return GetCVarCurrentByIndex<CVarType>(index).c_str();
	};

	void AutoCVar_String::Set(std::string&& val)
	{
		SetCVarCurrentByIndex<CVarType>(index, val);
	}


	void CVarSystemImpl::DrawImguiEditor()
	{
		static std::string searchText = "";

		ImGui::InputText("Filter", &searchText);
		static bool bShowAdvanced = false;
		ImGui::Checkbox("Advanced", &bShowAdvanced);
		ImGui::Separator();
		cachedEditParameters.clear();

		auto addToEditList = [&](auto parameter)
		{
			bool bHidden = ((uint32_t)parameter->flags & (uint32_t)CVarFlags::NoEdit);
			bool bIsAdvanced = ((uint32_t)parameter->flags & (uint32_t)CVarFlags::Advanced);

			if (!bHidden)
			{
				if (!(!bShowAdvanced && bIsAdvanced) && parameter->name.find(searchText) != std::string::npos)
				{
					cachedEditParameters.push_back(parameter);
				};
			}
		};

		for (int i = 0; i < GetCVarArray<int32_t>()->lastCVar; i++)
		{
			addToEditList(GetCVarArray<int32_t>()->cvars[i].parameter);
		}
		for (int i = 0; i < GetCVarArray<double>()->lastCVar; i++)
		{
			addToEditList(GetCVarArray<double>()->cvars[i].parameter);
		}
		for (int i = 0; i < GetCVarArray<std::string>()->lastCVar; i++)
		{
			addToEditList(GetCVarArray<std::string>()->cvars[i].parameter);
		}

		if (cachedEditParameters.size() > 10)
		{
			std::unordered_map<std::string, std::vector<CVarParameter*>> categorizedParams;

			//insert all the edit parameters into the hashmap by category
			for (auto p : cachedEditParameters)
			{
				int dotPos = -1;
				//find where the first dot is to categorize
				for (int i = 0; i < p->name.length(); i++)
				{
					if (p->name[i] == '.')
					{
						dotPos = i;
						break;
					}
				}
				std::string category = "";
				if (dotPos != -1)
				{
					category = p->name.substr(0, dotPos);
				}

				auto it = categorizedParams.find(category);
				if (it == categorizedParams.end())
				{
					categorizedParams[category] = std::vector<CVarParameter*>();
					it = categorizedParams.find(category);
				}
				it->second.push_back(p);
			}

			for (auto [category, parameters] : categorizedParams)
			{
				//alphabetical sort
				std::sort(parameters.begin(), parameters.end(), [](CVarParameter* A, CVarParameter* B)
					{
						return A->name < B->name;
					});

				if (ImGui::BeginMenu(category.c_str()))
				{
					float maxTextWidth = 0;

					for (auto p : parameters)
					{
						maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(p->name.c_str()).x);
					}
					for (auto p : parameters)
					{
						EditParameter(p, maxTextWidth);
					}

					ImGui::EndMenu();
				}
			}
		}
		else
		{
			//alphabetical sort
			std::sort(cachedEditParameters.begin(), cachedEditParameters.end(), [](CVarParameter* A, CVarParameter* B)
				{
					return A->name < B->name;
				});
			float maxTextWidth = 0;
			for (auto p : cachedEditParameters)
			{
				maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(p->name.c_str()).x);
			}
			for (auto p : cachedEditParameters)
			{
				EditParameter(p, maxTextWidth);
			}
		}
	}


	void Label(const char* label, float textWidth)
	{
		constexpr float Slack = 50;
		constexpr float EditorWidth = 100;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		const ImVec2 lineStart = ImGui::GetCursorScreenPos();
		const ImGuiStyle& style = ImGui::GetStyle();
		float fullWidth = textWidth + Slack;

		ImVec2 textSize = ImGui::CalcTextSize(label);

		ImVec2 startPos = ImGui::GetCursorScreenPos();

		ImGui::Text(label);

		ImVec2 finalPos = { startPos.x + fullWidth, startPos.y };

		ImGui::SameLine();
		ImGui::SetCursorScreenPos(finalPos);

		ImGui::SetNextItemWidth(EditorWidth);
	}


	void CVarSystemImpl::EditParameter(CVarParameter* p, float textWidth)
	{
		const bool readonlyFlag = ((uint32_t)p->flags & (uint32_t)CVarFlags::EditReadOnly);
		const bool checkboxFlag = ((uint32_t)p->flags & (uint32_t)CVarFlags::EditCheckbox);
		const bool dragFlag = ((uint32_t)p->flags & (uint32_t)CVarFlags::EditFloatDrag);


		switch (p->type)
		{
		case CVarType::INT:

			if (readonlyFlag)
			{
				std::string displayFormat = p->name + "= %i";
				ImGui::Text(displayFormat.c_str(), GetCVarArray<int32_t>()->GetCurrent(p->arrayIndex));
			}
			else
			{
				if (checkboxFlag)
				{
					bool bCheckbox = GetCVarArray<int32_t>()->GetCurrent(p->arrayIndex) != 0;
					Label(p->name.c_str(), textWidth);

					ImGui::PushID(p->name.c_str());

					if (ImGui::Checkbox("", &bCheckbox))
					{
						GetCVarArray<int32_t>()->SetCurrent(bCheckbox ? 1 : 0, p->arrayIndex);
					}
					ImGui::PopID();
				}
				else
				{
					Label(p->name.c_str(), textWidth);
					ImGui::PushID(p->name.c_str());
					ImGui::InputInt("", GetCVarArray<int32_t>()->GetCurrentPtr(p->arrayIndex));
					ImGui::PopID();
				}
			}
			break;

		case CVarType::FLOAT:

			if (readonlyFlag)
			{
				std::string displayFormat = p->name + "= %f";
				ImGui::Text(displayFormat.c_str(), GetCVarArray<double>()->GetCurrent(p->arrayIndex));
			}
			else
			{
				Label(p->name.c_str(), textWidth);
				ImGui::PushID(p->name.c_str());
				if (dragFlag)
				{
					ImGui::InputDouble("", GetCVarArray<double>()->GetCurrentPtr(p->arrayIndex), 0, 0, "%.3f");
				}
				else
				{
					ImGui::InputDouble("", GetCVarArray<double>()->GetCurrentPtr(p->arrayIndex), 0, 0, "%.3f");
				}
				ImGui::PopID();
			}
			break;

		case CVarType::STRING:

			if (readonlyFlag)
			{
				std::string displayFormat = p->name + "= %s";
				ImGui::PushID(p->name.c_str());
				ImGui::Text(displayFormat.c_str(), GetCVarArray<std::string>()->GetCurrent(p->arrayIndex));

				ImGui::PopID();
			}
			else
			{
				Label(p->name.c_str(), textWidth);
				ImGui::InputText("", GetCVarArray<std::string>()->GetCurrentPtr(p->arrayIndex));

				ImGui::PopID();
			}
			break;

		default:
			break;
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(p->description.c_str());
		}
	}












	using namespace CVarSystemImpl;


	CVarParameter* CVarSystem::GetCVar(StringUtils::StringHash hash)
	{
		return CVarSystemImpl::GetCVar(hash);
	}

	double* CVarSystem::GetFloatCVar(StringUtils::StringHash hash)
	{
		return CVarSystemImpl::GetFloatCVar(hash);
	}

	int32_t* CVarSystem::GetIntCVar(StringUtils::StringHash hash)
	{
		return CVarSystemImpl::GetIntCVar(hash);
	}

	const char* CVarSystem::GetStringCVar(StringUtils::StringHash hash)
	{
		return CVarSystemImpl::GetStringCVar(hash);
	}

	void CVarSystem::SetFloatCVar(StringUtils::StringHash hash, double value)
	{
		CVarSystemImpl::SetFloatCVar(hash, value);
	}

	void CVarSystem::SetIntCVar(StringUtils::StringHash hash, int32_t value)
	{
		CVarSystemImpl::SetIntCVar(hash, value);
	}

	void CVarSystem::SetStringCVar(StringUtils::StringHash hash, const char* value)
	{
		CVarSystemImpl::SetStringCVar(hash, value);
	}

	CVarParameter* CVarSystem::CreateFloatCVar(const char* name, const char* description, double defaultValue, double currentValue)
	{
		return CVarSystemImpl::CreateFloatCVar(name, description, defaultValue, currentValue);
	}

	CVarParameter* CVarSystem::CreateIntCVar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue)
	{
		return CVarSystemImpl::CreateIntCVar(name, description, defaultValue, currentValue);
	}

	CVarParameter* CVarSystem::CreateStringCVar(const char* name, const char* description, const char* defaultValue, const char* currentValue)
	{
		return CVarSystemImpl::CreateStringCVar(name, description, defaultValue, currentValue);
	}

	void CVarSystem::DrawImguiEditor()
	{
		CVarSystemImpl::DrawImguiEditor();
	}


	bool CVarSystem::Setup(ISystemConfig* config)
	{
		return true;
	}
	bool CVarSystem::Init()
	{
		return true;
	}
	bool CVarSystem::Update()
	{
		return true;
	}
	bool CVarSystem::Shutdown()
	{
		return true;
	}



}

