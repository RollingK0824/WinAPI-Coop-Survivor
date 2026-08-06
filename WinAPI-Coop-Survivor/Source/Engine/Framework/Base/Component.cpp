#include "Engine/Core/pch.h"
#include "Component.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Scene.h"

namespace
{
	const json* FindJsonField(const json& inJson, const std::string& name)
	{
		if (inJson.contains(name)) return &inJson[name];

		auto normalize = [](const std::string& s) {
			std::string result;
			for (char c : s)
			{
				if (isalnum(static_cast<unsigned char>(c)))
				{
					result += static_cast<char>(tolower(static_cast<unsigned char>(c)));
				}
			}
			return result;
		};

		std::string normName = normalize(name);
		for (auto it = inJson.begin(); it != inJson.end(); ++it)
		{
			std::string normKey = normalize(it.key());
			if (normKey == normName) return &it.value();
			if (normKey == normName + "id" || normKey + "id" == normName ||
				normKey == normName + "btnid" || normKey + "btn" == normName)
			{
				return &it.value();
			}
		}
		return nullptr;
	}
}

void Component::Serialize(json& outJson) const
{
	outJson[EngineKey::Property::IsEnabled.data()] = m_bIsEnabled;

	for (const auto& prop : m_vProperties)
	{
		if (!prop.data) continue;

		switch (prop.type)
		{
		case PropType::Int:
			outJson[prop.name] = *static_cast<int*>(prop.data);
			break;
		case PropType::Float:
			outJson[prop.name] = *static_cast<float*>(prop.data);
			break;
		case PropType::Bool:
			outJson[prop.name] = *static_cast<bool*>(prop.data);
			break;
		case PropType::String:
			outJson[prop.name] = *static_cast<std::string*>(prop.data);
			break;
		case PropType::WString:
		{
			std::wstring* wstr = static_cast<std::wstring*>(prop.data);
			outJson[prop.name] = std::string(wstr->begin(), wstr->end());
		}
		break;
		case PropType::Vector2:
		{
			Vector2* vec = static_cast<Vector2*>(prop.data);
			outJson[prop.name] = { {"x", vec->x}, {"y", vec->y} };
		}
		break;
		case PropType::Color:
		{
			D2D1_COLOR_F* col = static_cast<D2D1_COLOR_F*>(prop.data);
			outJson[prop.name] = { {"r", col->r}, {"g", col->g}, {"b", col->b}, {"a", col->a} };
		}
		break;
		case PropType::Rect:
		{
			D2D1_RECT_F* rect = static_cast<D2D1_RECT_F*>(prop.data);
			outJson[prop.name] = { {"left", rect->left}, {"top", rect->top}, {"right", rect->right}, {"bottom", rect->bottom} };
		}
		break;
		case PropType::StringVector:
			outJson[prop.name] = *static_cast<std::vector<std::string>*>(prop.data);
			break;
		case PropType::Texture:
		{
			std::wstring* wKey = static_cast<std::wstring*>(prop.data);
			outJson[prop.name] = std::string(wKey->begin(), wKey->end());
		}
		break;
		case PropType::ObjectRef:
		{
			uint64 id = prop.getInstanceID ? prop.getInstanceID(prop.data) : 0;
			outJson[prop.name] = id;
		}
		break;
		case PropType::Asset:
			outJson[prop.name] = *static_cast<uint32*>(prop.data);
			break;
		}
	}
}

void Component::Deserialize(const json& inJson)
{
	const json* enabledJson = FindJsonField(inJson, EngineKey::Property::IsEnabled.data());
	if (enabledJson && enabledJson->is_boolean())
	{
		m_bIsEnabled = enabledJson->get<bool>();
	}

	for (const auto& prop : m_vProperties)
	{
		if (!prop.data) continue;

		const json* pVal = FindJsonField(inJson, prop.name);
		if (!pVal) continue;

		switch (prop.type)
		{
		case PropType::Int:
			if (pVal->is_number()) *static_cast<int*>(prop.data) = pVal->get<int>();
			break;
		case PropType::Float:
			if (pVal->is_number()) *static_cast<float*>(prop.data) = pVal->get<float>();
			break;
		case PropType::Bool:
			if (pVal->is_boolean()) *static_cast<bool*>(prop.data) = pVal->get<bool>();
			break;
		case PropType::String:
			if (pVal->is_string()) *static_cast<std::string*>(prop.data) = pVal->get<std::string>();
			break;
		case PropType::WString:
		{
			if (pVal->is_string())
			{
				std::string str = pVal->get<std::string>();
				*static_cast<std::wstring*>(prop.data) = std::wstring(str.begin(), str.end());
			}
		}
		break;
		case PropType::Vector2:
		{
			Vector2* vec = static_cast<Vector2*>(prop.data);
			if (pVal->is_object())
			{
				if (pVal->contains("x")) vec->x = (*pVal)["x"].get<float>();
				if (pVal->contains("y")) vec->y = (*pVal)["y"].get<float>();
			}
		}
		break;
		case PropType::Color:
		{
			D2D1_COLOR_F* col = static_cast<D2D1_COLOR_F*>(prop.data);
			if (pVal->is_object())
			{
				if (pVal->contains("r")) col->r = (*pVal)["r"].get<float>();
				if (pVal->contains("g")) col->g = (*pVal)["g"].get<float>();
				if (pVal->contains("b")) col->b = (*pVal)["b"].get<float>();
				if (pVal->contains("a")) col->a = (*pVal)["a"].get<float>();
			}
		}
		break;
		case PropType::Rect:
		{
			D2D1_RECT_F* rect = static_cast<D2D1_RECT_F*>(prop.data);
			if (pVal->is_object())
			{
				if (pVal->contains("left")) rect->left = (*pVal)["left"].get<float>();
				if (pVal->contains("top")) rect->top = (*pVal)["top"].get<float>();
				if (pVal->contains("right")) rect->right = (*pVal)["right"].get<float>();
				if (pVal->contains("bottom")) rect->bottom = (*pVal)["bottom"].get<float>();
			}
		}
		break;
		case PropType::StringVector:
		{
			if (pVal->is_array())
			{
				*static_cast<std::vector<std::string>*>(prop.data) = pVal->get<std::vector<std::string>>();
			}
		}
		break;
		case PropType::Texture:
		{
			if (pVal->is_string())
			{
				std::string str = pVal->get<std::string>();
				*static_cast<std::wstring*>(prop.data) = std::wstring(str.begin(), str.end());
			}
		}
		break;
		case PropType::ObjectRef:
		{
			if (pVal->is_number())
			{
				m_pendingObjectRefs[prop.name] = pVal->get<uint64>();
			}
			else if (pVal->is_object() && pVal->contains("InstanceID"))
			{
				m_pendingObjectRefs[prop.name] = (*pVal)["InstanceID"].get<uint64>();
			}
		}
		break;
		case PropType::Asset:
			if (pVal->is_number()) *static_cast<uint32*>(prop.data) = pVal->get<uint32>();
			break;
		}
	}
}

void Component::PostDeserialize(Scene* pScene)
{
	for (const auto& prop : m_vProperties)
	{
		if (prop.type == PropType::ObjectRef)
		{
			auto it = m_pendingObjectRefs.find(prop.name);
			if (it != m_pendingObjectRefs.end())
			{
				uint64 targetID = it->second;
				if (targetID != 0 && pScene != nullptr)
				{
					GameObject* targetObj = pScene->FindGameObjectByInstanceID(targetID);
					if (prop.resolver)
					{
						prop.resolver(targetObj, prop.data);
					}
				}
				else
				{
					if (prop.resolver)
					{
						prop.resolver(nullptr, prop.data);
					}
				}
			}
		}
	}
	m_pendingObjectRefs.clear();
}
